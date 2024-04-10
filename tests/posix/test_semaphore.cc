#include <vector>
#include <pthread.h>
#include "gtest/gtest.h"

#include "libosal/osal.h"
#include "test_utils.h"
#include <pthread.h>

namespace test_mutex {
  
  using testutils::wait_nanoseconds;


  /* the following test runs two threads, a sender
     and a receiver, each of which references a common
     semaphore, and each of which use a shared counter.

     Each thread loops. In each iteration, both will pause a random
     time. The sender registers the time and posts the semaphore, and
     the reveiver waits to receive it, and registers the time.

     The rationale of the test is that if the semaphore would not
     ensure ordering, the receive times would be before post times.
  */
  const uint LOOPCOUNT = 10000;

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
     these.
  */
     
  typedef struct {
    int thread_id;
    osal_semaphore_t sema;
    unsigned long value;
    bool wait_before_read;
    bool was_read; // flag to signal finished iteration
    pthred_mutex_t wasread_mutex;
    pthread_cond_t wasread_cond; // condition variable for signaling read
    
    struct timespec startwait_times[LOOPCOUNT];
    struct timespec read_times[LOOPCOUNT];
    unsigned long read_values[LOOPCOUNT];
  } thread_param_t;
  
  void* test_random(void *p_params)
  {
    // max time for random wait
    const int MAX_WAIT_TIME_NS = 1000;
    
    thread_param_t params = *((thread_param_t*) p_params);

    const int thread_id = params.thread_id;
    const uint max_wait_time = params.max_wait_time_nsec;
    

    // initialize PRNG
    srand(1);
    
    for(uint i = 0; i < params.loopcount; i++){

      // randomly wait, if flag checked
      if (params.wait_before_read) {
	wait_nanoseconds(rand() % max_wait_time);
      }
      // store the beginning of wait timestamp
      clock_gettime(CLOCK_MONOTONIC, params.startwait_times[i]);

      // wait for semaphore
      osal_semaphore_wait(&params.sema);
      // store read time
      clock_gettime(CLOCK_MONOTONIC, params.read_times[i]);

      // store the value passed from the sender
      params.read_values[i] = params.value;
      
      // now, we need to signal back to the sender
      // that we are done reading - we use
      // a pthread-locked condition variable for this,
      // since we do not want to rely on functions
      // which are tested.
      pthread_mutex_lock(&params.wasread_mutex);
      params.was_read = true;
      pthread_cond_signal(params.wasread_cond);
      pthread_mutex_unlock(params.wasread_mutex);

    }
  
  
  return nullptr;
}
  


TEST(Semaphore, RandomizedWait)
{
  const uint MAX_WAIT_TIME_NSEC = 500;

  
  pthread_t thread_id;
  
  thread_param_t params; /* shared data protected by
			    semaphore and mutex */
  
  struct timespec send_times[LOOPCOUNT];
  

  bool verbose = (getenv("VERBOSE") != nullptr);

  pthread_mutex_create(&params.wasread_mutex, nullptr);
  pthread_cond_init(&params.wasread_cond, nullptr);
  params.wait_before_read = false;


  osal_semaphore_init(&params.sema, nullptr, 0);
  
  pthread_create(/*thread*/ &(thread_id),
		 /*pthread_attr*/ nullptr,
		 /* start_routine */ test_semaphore,
		 /* arg */ (void*) &params);

  srand(1);

  for (ulong i=0; i < LOOPCOUNT; i++){
    // generate and send random value
    params.value  = rand();
    // store the time
    clock_gettime(CLOCK_MONOTONIC, send_times[i]);
    // signal via semaphore to receiver
    osal_semaphore_post(&params.sema);

    
    // now, wait for the "read complete" signal.
    // since we cannot rely on the semaphore
    // here, we use the shared pthreads mutex/
    // condition variable.
    pthread_mutex_lock(params.p_mutex);
    while (!params.wasread) {
      pthread_cond_wait(&params.mutex, &params.cond)
	}
    params.wasread=false;
    pthread_mutex_unlock(params.p_mutex);
  }

  pthread_join(/*thread*/ thread_id,
	       /*retval*/ nullptr);
  
  pthread_cond_destroy(&params.wasread_cond);
  pthread_mutex_destroy(&params.wasread_mutex);

  EXPECT_EQ(counter, N_THREADS * LOOPCOUNT)
    << "multi-threaded counter test failed";

  
}

  
}


int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
