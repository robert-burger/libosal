#include <vector>
#include <pthread.h>
#include "gtest/gtest.h"

#include "libosal/osal.h"
#include "test_utils.h"

namespace test_mutex {
  
  using testutils::wait_nanoseconds;

  TEST(MutexSane, SingleThreadedNoRelease)
  {
    osal_mutex_t my_mutex;
    osal_mutex_init(&my_mutex, nullptr);
    osal_mutex_lock(&my_mutex);
    const int loopcount = 100;
    int counter = 0;
    
    for(int i = 0; i < loopcount; i++){
      counter += 1;
    }
    
    osal_mutex_unlock(&my_mutex);
    
    EXPECT_EQ(counter, loopcount) << (" sanity test failed, something "
				      "is totally wrong");
  }
  
  TEST(MutexSane, SingleThreadedWithRelease)
  {
    osal_mutex_t my_mutex;
    osal_mutex_init(&my_mutex, nullptr);
    const int loopcount = 100;
    int counter = 0;
    
    for(int i = 0; i < loopcount; i++){
      osal_mutex_lock(&my_mutex);
      counter += 1;
      osal_mutex_unlock(&my_mutex);
    }
    
    EXPECT_EQ(counter, loopcount) << (" sanity test failed, something "
				    "is totally wrong");
  }

  /* the following test runs N threads, each of which hold a common
     mutex, and each of which use a common counter.

     Each thread loops, and in each iteration, will pause a random
     time (in order to increase chance of concurrent access), get the
     mutex, pause a bit more, increase the coutner, and return the
     mutex.

     The rationale of the test is that if the mutex would not
     protect shared data properly, we would have some missing
     counts due to race conditions.
  */
  typedef struct {
    int thread_id;
    uint loopcount;
    uint max_wait_time_nsec;
    unsigned long *p_counter;
    osal_mutex_t *p_count_mutex;
  } thread_param_t;
  
  void* test_random(void *p_params)
  {
    thread_param_t params = *((thread_param_t*) p_params);
    
    const int thread_id = params.thread_id;
    const uint max_wait_time = params.max_wait_time_nsec;
    

    if (max_wait_time > 0) {
      srand(thread_id);
    }
    for(uint i = 0; i < params.loopcount; i++){

      // randomly wait
      if ((max_wait_time > 0) && (rand() % 2)) {
	wait_nanoseconds(rand() % max_wait_time);
      }
      
      osal_mutex_lock(params.p_count_mutex);

      unsigned long old_value = *(params.p_counter);
      //randomly wait
      if ((max_wait_time > 0) && (rand() % 2)){
	wait_nanoseconds(rand() % max_wait_time);
      }

      // increment shared counter
      *(params.p_counter) = old_value + 1;

      //return lock
      osal_mutex_unlock(params.p_count_mutex);
    }
  
  
  return nullptr;
}
  
TEST(MutexMultithreading, Randomized)
{
  const ulong N_THREADS = 8;
  const uint LOOPCOUNT = 10000;

  
  pthread_t thread_ids[N_THREADS];
  thread_param_t thread_params[N_THREADS];
  osal_mutex_t count_mutex;
  unsigned long counter = 0;

  bool verbose = (getenv("VERBOSE") != nullptr);

  osal_mutex_init(&count_mutex, nullptr);

  for (ulong i = 0; i < N_THREADS; i++){
    thread_params[i].thread_id = i;
    thread_params[i].p_count_mutex = &count_mutex;
    thread_params[i].p_counter = &counter;
    thread_params[i].loopcount = LOOPCOUNT;
    thread_params[i].max_wait_time_nsec = 0;

    if (verbose){
      printf("starting thread %lu\n", i);
    }
    pthread_create(/*thread*/ &(thread_ids[i]),
		   /*pthread_attr*/ nullptr,
		   /* start_routine */ test_random,
		   /* arg */ (void*) &(thread_params[i]));
    
  }
  for (ulong i = 0; i < N_THREADS; i++){
    if (verbose) {
      printf("joining thread %lu\n", i);
    }
    pthread_join(/*thread*/ thread_ids[i],
		 /*retval*/ nullptr);
  }
  if (verbose) {
    printf("expected counts: %lu, actual counter: %lu \n",
	   N_THREADS * LOOPCOUNT,
	   counter);
  }
  
  EXPECT_EQ(counter, N_THREADS * LOOPCOUNT)
    << "multi-threaded counter test failed";
  	
}


TEST(MutexMultithreading, RandomizedPlusWait)
{
  const ulong N_THREADS = 8;
  const uint LOOPCOUNT = 10000;
  const uint MAX_WAIT_TIME_NSEC = 500;

  
  pthread_t thread_ids[N_THREADS];
  thread_param_t thread_params[N_THREADS];
  osal_mutex_t count_mutex;
  unsigned long counter = 0;

  bool verbose = (getenv("VERBOSE") != nullptr);

  osal_mutex_init(&count_mutex, nullptr);

  for (ulong i = 0; i < N_THREADS; i++){
    thread_params[i].thread_id = i;
    thread_params[i].p_count_mutex = &count_mutex;
    thread_params[i].p_counter = &counter;
    thread_params[i].loopcount = LOOPCOUNT;
    thread_params[i].max_wait_time_nsec = MAX_WAIT_TIME_NSEC;

    if (verbose){
      printf("starting thread %lu\n", i);
    }
    pthread_create(/*thread*/ &(thread_ids[i]),
		   /*pthread_attr*/ nullptr,
		   /* start_routine */ test_random,
		   /* arg */ (void*) &(thread_params[i]));
    
  }
  // should complete in about 50 ms
  for (ulong i = 0; i < N_THREADS; i++){
    if (verbose) {
      printf("joining thread %lu\n", i);
    }
    pthread_join(/*thread*/ thread_ids[i],
		 /*retval*/ nullptr);
  }
  if (verbose) {
    printf("expected counts: %lu, actual counter: %lu \n",
	   N_THREADS * LOOPCOUNT,
	   counter);
  }
  
  EXPECT_EQ(counter, N_THREADS * LOOPCOUNT)
    << "multi-threaded counter test failed";
  	
}

  
}
