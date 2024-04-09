#include <time.h>
#include <vector>
#include <pthread.h>
#include "gtest/gtest.h"

#include "libosal/timer.h"
#include "libosal/osal.h"
#include <sched.h>

namespace test_timer {
using std::vector;    

bool is_realtime()
{
  bool runs_realtime = false;
#if __linux__
  if (sched_getscheduler(0) == SCHED_FIFO) {
    runs_realtime = true;
  }
#endif
  return runs_realtime;
}

void wait_nanoseconds(long wait_time)
{
  int ret_val = -1;
  struct timespec req_wait = {0, wait_time};
  
  errno = 0;
  while (ret_val != 0) {
    struct timespec remain_wait = {0,0};
    ret_val = nanosleep(&req_wait, &remain_wait);
    if ((ret_val == -1) and (errno == EINTR)){
      req_wait = remain_wait;
      continue;
    }
  }
}

/* the goal of the following test is NOT
   to check that strict latency requirements
   are passed. The test is only meant as
   a basic sanity / smoke test to check
   whether the OSAL API works at all.
*/

uint64_t measure_timer(long req_wait_time_nsecs)
{
  struct timespec tp1;
  struct timespec tp2;
  struct osal_timer timer;
  clock_gettime(CLOCK_MONOTONIC, &tp1);

  osal_timer_init(&timer, req_wait_time_nsecs);


  while (true){

    // wait one microsecond
    wait_nanoseconds(1000);
    
    osal_retval_t orv = osal_timer_expired(&timer);
    if (orv == OSAL_ERR_TIMEOUT){
      break;
    }
  };
  clock_gettime(CLOCK_MONOTONIC, &tp2);
  
  // get measured system time difference
  uint64_t time_diff_nsecs = ((tp2.tv_sec - tp1.tv_sec) * 1000000000
			   + (tp2.tv_nsec - tp1.tv_nsec));

  return time_diff_nsecs;

}

void check_wait_times(vector<osal_uint64_t> &req_wait_times,
		      bool runs_realtime,
		      uint64_t less_margin,
		      uint64_t more_margin)
{
  for (osal_uint64_t req_wait_time_nsecs : req_wait_times){

    uint64_t run_time_nsecs = measure_timer(req_wait_time_nsecs);
    
    printf("runs realtime: %s \n", runs_realtime ? "yes" : "no"); 
    printf("requested time: %lu nsec, actual wait: %lu nsec\n",
	   (ulong) req_wait_time_nsecs,
	   (ulong) run_time_nsecs);

    

    EXPECT_GE(run_time_nsecs, req_wait_time_nsecs + less_margin) << "measured time has negative difference from requested time";
    EXPECT_LE(run_time_nsecs, req_wait_time_nsecs + more_margin) << "Timer tardiness exceeds tolerance";
  }

}

TEST(Timer, SaneSingleThreaded)
{
  const bool runs_realtime = is_realtime();

  // we allow no negative time tifference,
  // but a positive time difference of 10 μs when
  // running with real-time scheduling, and
  // 100 μs otherwise.
  const int64_t TIMER_TOLERANCE_LESS_NS = 0;
  const int64_t TIMER_TOLERANCE_MORE_NS = runs_realtime ? 10000 : 100000;

  
  vector<osal_uint64_t> req_wait_times = {500000000,
					  50000000,
					  5000000,
					  500000,
					  50000,
					  5000};

  check_wait_times(req_wait_times,
		   runs_realtime,
		   TIMER_TOLERANCE_LESS_NS,
		   TIMER_TOLERANCE_MORE_NS);	
}

  /* helper function used to randomize wait times */

  vector<osal_uint64_t> shuffle_vector(vector<osal_uint64_t> ordered_numbers,
				       int seed)
  {
    vector<osal_uint64_t> vec(ordered_numbers); // allocate a copy

    size_t vlen = vec.size();
    srand(seed);
    // shuffling elements with Fisher-Yates shuffle
    // see https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
    for(size_t i = 0; i < (vlen - 1); i++){
      size_t new_pos = i + rand() % (vlen - i);
      std::swap(vec[i], vec[new_pos]);
    }
    return vec;
  }

  /* set up to run timer waits with shuffled times
     in multiple threads */
  
  typedef struct {
    vector<osal_uint64_t> wait_times;
    bool runs_realtime;
    int64_t less_margin;
    int64_t more_margin;
  } wait_params_t;
  
  void* check_wait_times_mt(void* vparams)
  {
    wait_params_t * p_params = (wait_params_t *) vparams;
    wait_params_t params  = *p_params;
    check_wait_times(params.wait_times,
		     params.runs_realtime,
		     params.less_margin,
		     params.more_margin);	
    return nullptr;
  }

TEST(Timer, SaneMultiThreaded)
{
  const bool runs_realtime = is_realtime();

  // we allow no negative time tifference,
  // but a positive time difference of 10 μs when
  // running with real-time scheduling, and
  // 100 μs otherwise.
  const int64_t TIMER_TOLERANCE_LESS_NS = 0;
  const int64_t TIMER_TOLERANCE_MORE_NS = runs_realtime ? 10000 : 100000;
  //const int64_t TIMER_TOLERANCE_MORE_NS = 100;
  
  const int N_THREADS = 10;
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
  vector<wait_params_t> thread_params_vec;
  

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
    const wait_params_t params = {shuffled_times,
				  runs_realtime,
				  TIMER_TOLERANCE_LESS_NS,
				  TIMER_TOLERANCE_MORE_NS};
    thread_params_vec.push_back(params);
  }
  for (int i = 0; i < N_THREADS; i++){
    printf("starting thread %i\n", i);
    pthread_create(/*thread*/ &(thread_ids[i]),
		   /*pthread_attr*/ nullptr,
		   /* start_routine */ check_wait_times_mt,
		   /* arg */ (void*) &(thread_params_vec[i]));
    
  }
  for (int i = 0; i < N_THREADS; i++){
    printf("joining thread %i\n", i);
    pthread_join(/*thread*/ thread_ids[i],
		 /*retval*/ nullptr);
  }
  	
}

}
