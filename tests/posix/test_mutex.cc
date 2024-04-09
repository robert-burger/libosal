#include <vector>
#include <pthread.h>
#include "gtest/gtest.h"

//#include "libosal/timer.h"
#include "libosal/osal.h"
#include "test_utils.h"

namespace test_timer {
using std::vector;
  //using testutils::wait_nanoseconds;
  //using testutils::shuffle_vector;

TEST(MutexSane, SingleThreaded)
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
  

#if 0

TEST(TimerSleepUntil, SaneMultiThreaded)
{
  const bool runs_realtime = is_realtime();

  // we allow no negative time tifference,
  // but a positive time difference of 10 μs when
  // running with real-time scheduling, and
  // 100 μs otherwise.
  const int64_t TIMER_TOLERANCE_LESS_NS = 0;
  const int64_t TIMER_TOLERANCE_MORE_NS = runs_realtime ? 100000 : 150000;
  //const int64_t TIMER_TOLERANCE_MORE_NS = 100;
  
  const int N_THREADS = 8;
  pthread_t thread_ids[N_THREADS];
  
  vector<osal_uint64_t> req_wait_times = {500000000,
					  200000000,
					  100000000,
					  50000000,
					  20000000,
					  10000000,
					  5000000,
					  2000000,
					  1000000,
					  500000,
					  200000,
					  100000,
					  50000,
					  20000,
					  10000,
					  5000,
					  2000,
					  1000};

  // because pthread_start passes only a single
  // argument pointer, we store parameters in
  // a struct.
  vector<sleep_until_params_t> thread_params_vec;
  

  // randomize wait times and parameters
  // We do this ahead and keep each result
  // as a copy because we need to keep the
  // data constant while the threads
  // are running.
  for (int i = 0; i < N_THREADS; i++){
    // create random-shuffled copy of wait times
    const int seed_value = i;
    vector<osal_uint64_t> shuffled_times = shuffle_vector(req_wait_times,
							  seed_value);
    // store parameters
    const sleep_until_params_t params = {shuffled_times,
					 runs_realtime,
					 TIMER_TOLERANCE_LESS_NS,
					 TIMER_TOLERANCE_MORE_NS};
    thread_params_vec.push_back(params);
  }
  for (int i = 0; i < N_THREADS; i++){
    printf("starting thread %i\n", i);
    pthread_create(/*thread*/ &(thread_ids[i]),
		   /*pthread_attr*/ nullptr,
		   /* start_routine */ check_sleep_until_mt,
		   /* arg */ (void*) &(thread_params_vec[i]));
    
  }
  for (int i = 0; i < N_THREADS; i++){
    printf("joining thread %i\n", i);
    pthread_join(/*thread*/ thread_ids[i],
		 /*retval*/ nullptr);
  }
  	
}

#endif
  
}
