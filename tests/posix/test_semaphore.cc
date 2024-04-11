#include "gtest/gtest.h"
#include "libosal/osal.h"
#include "libosal/timer.h"
#include "test_utils.h"
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <vector>
#include <cassert>

namespace test_semaphore {

  static int verbose = 0;

  using testutils::wait_nanoseconds;
  using testutils::is_realtime;


  /* the following test runs two threads, a sender
     and a receiver, each of which references a common
     semaphore, and each of which use a shared counter.

     Each thread loops. In each iteration, the receiver may pause a
     random time. The sender registers the time and posts to the
     semaphore, and the reveiver waits to receive it, and upton
     receival registers that time.  Also, the sender writes a random
     number to shared memory and stores it in a private array, and the
     receiver stores the value in another array.

     The rationale of the test is that if the semaphore would not
     ensure ordering, the receive times would be before post times.
     Further, the stored random numbers need to match, if the
     tested semaphore in fact prevents race conditions.
  */
  const uint LOOPCOUNT = 50000;

  /* The struct here is shard data used for the test.  One could
     wonder why it has both a smaphore AND a mutex - isn't this a bit
     of overkill?
     
     The answer is: The semaphore is being tested, so we cannot rely
     on it.

     But on the other hand, apart from send/receive times, we also
     test that the semaphore defines before/after semantics for a
     shared value. For testing that this before/after is correct, we
     need to make sure that the sender thread gets a correct
     notification that it can send new values before overwriting
     the values sent before.
  */
     
  typedef struct {
    osal_semaphore_t sema;
    unsigned long value;
    bool wait_before_read;
    bool was_read; // flag to signal finished iteration
    pthread_mutex_t wasread_mutex;
    pthread_cond_t wasread_cond; // condition variable for signaling read
    
    struct timespec startwait_times[LOOPCOUNT];
    struct timespec read_times[LOOPCOUNT];
    unsigned long read_values[LOOPCOUNT];
  } thread_param_t;
  
  void* test_semaphore(void *p_params)
  {
    // max time for random wait
    const int MAX_WAIT_TIME_NS = 1000;

    
    assert(p_params != nullptr);
    // keep in mind that params is necessarily shared here,
    // differently from some other test code.
    thread_param_t *params = ((thread_param_t*) p_params);    
    // initialize PRNG
    srand(1);
    osal_retval_t orv= {};
    int rv = {};
    for(uint i = 0; i < LOOPCOUNT; i++){

      // randomly wait, if flag checked
      if (params->wait_before_read) {
	wait_nanoseconds(rand() % MAX_WAIT_TIME_NS);
      }
      // store the beginning of wait timestamp
      clock_gettime(CLOCK_MONOTONIC, &params->startwait_times[i]);

      // wait for semaphore
      if (verbose) {
	printf("[%u] receiver: waiting for sema\n", i);
      }
      orv = osal_semaphore_wait(&params->sema);
      // note: this and the following are not assertions
      // because it does not work.... seems that
      // the ASSERT macros contain a return which does
      // not work for calles functions, while EXPECT_* does.
      EXPECT_EQ(orv, OSAL_OK) << "error in osal_semaphore_wait()";

      if (verbose) {
	printf("[%u] receiver: got sema\n", i);
      }
      // store read time
      rv = clock_gettime(CLOCK_MONOTONIC, &params->read_times[i]);
      EXPECT_EQ(rv, 0);

      // store the value passed from the sender
      params->read_values[i] = params->value;
      
      // now, we need to signal back to the sender
      // that we are done reading - we use
      // a pthread-locked condition variable for this,
      // since we do not want to rely on functions
      // which are tested.
      if (verbose) {
	printf("[%u] receiver: updating flag\n", i);
      }
      rv = pthread_mutex_lock(&params->wasread_mutex);
      EXPECT_EQ(rv, 0) << "could not lock mutex";
      params->was_read = true;
      if (verbose) {
	printf("[%u] receiver: flag was set\n", i);
      }
      rv = pthread_cond_signal(&params->wasread_cond);
      EXPECT_EQ(rv, 0) << "signaling condition failed";
      rv = pthread_mutex_unlock(&params->wasread_mutex);
      EXPECT_EQ(rv, 0) << "could not unlock mutex";
      if (verbose) {
	printf("[%u] receiver: update done\n", i);
      }

    }
  
  
  return nullptr;
}
  


    TEST(Semaphore, RandomizedWait)
    {
      const uint64_t MAX_LAG_REALTIME_NSEC = 50000;
      const uint64_t MAX_LAG_BATCH_NSEC = 100000;

      
      pthread_t thread_id;
      thread_param_t params; /* shared data protected by
    			    semaphore and mutex */
      struct timespec send_times[LOOPCOUNT];  
      unsigned long send_values[LOOPCOUNT];
      
      // we initialize the received values, so that
      // the variables are still initialized in case of a
      // test failure.
      memset(params.read_values, 0, sizeof(params.read_values));
      memset(params.read_times, 0, sizeof(params.read_times));
      memset(send_times, 0, sizeof(send_times));
      memset(send_values, -1, sizeof(send_values));

      osal_retval_t orv;
      int rv = pthread_mutex_init(&params.wasread_mutex, nullptr);
      ASSERT_EQ(rv, 0) << " could not create mutex";
      rv = pthread_cond_init(&params.wasread_cond, nullptr);
      ASSERT_EQ(rv, 0) << " could not create cond var";
    
      // flag whether the reader sleeps extra time
      // before waiting for the semaphore.
      //
      // This is added to ensure that
      // no signals are lost when the senders post() call
      // was sent before the readers wait.
      params.wait_before_read = false;
      params.was_read = false;    

      osal_semaphore_attr_t attr = OSAL_SEMAPHORE_ATTR__PROCESS_SHARED;
      orv = osal_semaphore_init(&params.sema, &attr, 0);
      ASSERT_EQ(orv, OSAL_OK) << "osal_semaphore_init() failed";

      
      srand(1);
      rv = pthread_create(/*thread*/ &(thread_id),
			  /*pthread_attr*/ nullptr,
			  /* start_routine */ test_semaphore,
			  /* arg */ (void*) &params);
      ASSERT_EQ(rv, 0) << "pthread_create() failed";
      
    

      for (uint i=0; i < LOOPCOUNT; i++){
        // generate and send random value
        unsigned long val = rand();
        params.value  = val;
        send_values[i] = val;
        // store the time, to compare it later
        rv = clock_gettime(CLOCK_MONOTONIC, &send_times[i]);
	ASSERT_EQ(rv, 0) << "clock_gettime() failed";
        // signal via semaphore to receiver
	if (verbose) {
  printf("[%u] sender: posting to semaphore\n", i);
}
        orv = osal_semaphore_post(&params.sema);
	ASSERT_EQ(orv, OSAL_OK) << "osal_semaphore_post() failed";
    
        
        // now, wait for the "read complete" signal.  Completion of
	// the read is not ensured by the semaphore. Since we cannot
	// yet rely on the semaphore tested here to work correctly, we
	// use the shared pthreads mutex/ condition variable.
	if (verbose) {
	  printf("[%u] sender: locking cvar\n", i);
	}
        rv = pthread_mutex_lock(&params.wasread_mutex);
	ASSERT_EQ(rv, 0) << "mutex lock failed";
        while (!params.was_read) {
          // we wait for the reader thread to respond,
          // but error out when there is no response
          // after MAX_WAIT_SEC seconds.
	  if (verbose) {
  printf("[%u] sender: cond false, waiting\n", i);
}
          const int MAX_WAIT_SEC = 5;
          timespec wait_time = {};
	  wait_time.tv_sec = time(nullptr) + MAX_WAIT_SEC;
	  if (verbose) {
	    printf("waiting maximally until %lu sec epoch\n",
		   wait_time.tv_sec);
	  }
	  wait_time.tv_nsec = 0;      
          rv = pthread_cond_timedwait(&params.wasread_cond,
				      &params.wasread_mutex,
				      &wait_time);
          ASSERT_EQ(rv, 0) << "pthread_cond_[timed]wait() failed";
        }
        params.was_read=false;
        rv = pthread_mutex_unlock(&params.wasread_mutex);
	ASSERT_EQ(rv, 0) << "pthread_mutex_unlock() failed";

	if (verbose) {
	  printf("[%u] sender: proceeding\n", i);
	}
      }

      rv = pthread_join(/*thread*/ thread_id,
			/*retval*/ nullptr);
      ASSERT_EQ(rv, 0) << "pthread_join() failed";

    
      rv = pthread_cond_destroy(&params.wasread_cond);
      ASSERT_EQ(rv, 0) << "could not destroy cond var";
      
      rv = pthread_mutex_destroy(&params.wasread_mutex);
      ASSERT_EQ(rv, 0) << "could not destroy mutex";
      
      
      // Now, we have the send times in send_times[],
      // and the read times in params.read_times[]
      // Equally, we have the values that were
      // written to the shared memory in send_values[],
      // and the received values in parms.read_values[].
    
      // The values need to match, and the read time
      // must never be smaller than the send time,
      // otherwise the semaphore would be broken.
    
      for (ulong i=0; i < LOOPCOUNT; i++){
	EXPECT_EQ(send_values[i], params.read_values[i])
          << "sent and received values do not match";
      }

      int64_t max_lag = (is_realtime()?
			 MAX_LAG_REALTIME_NSEC :
			 MAX_LAG_BATCH_NSEC);
	 

      // We do a sanity check for the times -
      // Again, this is not meant as a check of real-time
      // performace, but just whether the implementation
      // is sane.
      // We require:
      // 1. receive times must never be before send times
      // 2. the time difference should not be too large
      
      for (ulong i=0; i < LOOPCOUNT; i++){
        timespec tp1 = send_times[i];
        timespec tp2 = params.read_times[i];
	//  cast here because the difference can be
	// negative, and tv_sec is unsigned.
        int64_t time_diff_nsecs = ((((int64_t)tp2.tv_sec)
				    - ((int64_t)tp1.tv_sec)) * 1000000000
				   + (tp2.tv_nsec - tp1.tv_nsec));
    
        EXPECT_GE(time_diff_nsecs, 0)
          << "the read time was ahead of the send time";
        EXPECT_LT(time_diff_nsecs, max_lag)
          << "the time difference between wait() and send() was too large";
      }
      
    }
}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);

  if (getenv("VERBOSE")){
    test_semaphore::verbose=1;
  }
  // try to lock memory
  errno = 0;
  if (mlockall(MCL_CURRENT|MCL_FUTURE) == -1){
      perror("test_semaphore: could not lock memory"); 
    }
    return RUN_ALL_TESTS();
}
