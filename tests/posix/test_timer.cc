#include <time.h>
#include <vector>

#include "gtest/gtest.h"

#include "libosal/timer.h"
#include "libosal/osal.h"
#include <sched.h>

bool is_realtime()
{
  bool runs_realtime = false;
  if (sched_getscheduler(0) == SCHED_FIFO) {
    runs_realtime = true;
  }
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

TEST(Timer, FailExpected)
{
  const bool runs_realtime = is_realtime();

  // we allow no negative time tifference,
  // but a positive time difference of 5 μs when
  // running with real-time scheduling, and
  // 100 μs otherwise.
  const int64_t TIMER_TOLERANCE_LESS_NS = 0;
  const int64_t TIMER_TOLERANCE_MORE_NS = runs_realtime ? 5000 : 100000;

  using std::vector;    
  
  vector<osal_uint64_t> req_wait_times = {500000000,
					  50000000,
					  5000000,
					  500000,
					  50000,
					  5000};

  for (osal_uint64_t req_wait_time_nsecs : req_wait_times){

    uint64_t run_time_nsecs = measure_timer(req_wait_time_nsecs);

    printf("runs realtime: %s \n", runs_realtime ? "yes" : "no"); 
    printf("requested time: %lu nsec, actual wait: %lu nsec\n",
	   (ulong) req_wait_time_nsecs,
	   (ulong) run_time_nsecs);

    

    EXPECT_GE(run_time_nsecs, req_wait_time_nsecs + TIMER_TOLERANCE_LESS_NS) << "measured time has negative difference from requested time";
    EXPECT_LE(run_time_nsecs, req_wait_time_nsecs + TIMER_TOLERANCE_MORE_NS) << "Timer tardiness exceeds tolerance";
  }
	
}

