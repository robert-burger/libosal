#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <vector>
#include "gtest/gtest.h"

#include <sched.h>
#include "libosal/osal.h"
#include "libosal/timer.h"
#include "test_utils.h"

namespace test_timer {
using std::vector;
using testutils::is_realtime;
using testutils::shuffle_vector;
using testutils::wait_nanoseconds;

/* EXPLANATION TO TESTS

   These tests have all the same basic structure:

   1. a timer or sleep command is configured and initialized
   2. we measure the start time
   3. we wait until the timer expires
   4. we measure the stop time
   5. we compare start and stop time according to the
      following criterions:

   a. a timer should never expire before it is due
   b. we give some tolerance for the end time
   c. if we are not under real-time scheduling,
      we increase the tolerance.

  The variantions cover whether it is a blocking sleep,
  a background timer, or a command that waits until a given
  time.

  We test both single-threaded and with multiple threads.
  For multi-threaded tests, we use an assortment of
  wait times, which we pseudo-randomly shuffle between
  threads.
*/

static int verbose = 0;
static int check_latency = 0;

/* the goal of the following test is NOT
   to check that strict latency requirements
   are passed. The test is only meant as
   a basic sanity / smoke test to check
   whether the OSAL API works at all.
*/

uint64_t measure_timer(long req_wait_time_nsecs) {
  struct timespec tp1;
  struct timespec tp2;
  osal_timer_t timer;
  clock_gettime(CLOCK_MONOTONIC, &tp1);

  osal_timer_init(&timer, req_wait_time_nsecs);

  while (true) {
    // wait one microsecond
    wait_nanoseconds(1000);

    osal_retval_t orv = osal_timer_expired(&timer);
    if (orv == OSAL_ERR_TIMEOUT) {
      break;
    }
  };
  clock_gettime(CLOCK_MONOTONIC, &tp2);

  // get measured system time difference
  uint64_t time_diff_nsecs =
      ((tp2.tv_sec - tp1.tv_sec) * 1000000000 + (tp2.tv_nsec - tp1.tv_nsec));

  return time_diff_nsecs;
}

void check_wait_times(vector<osal_uint64_t> &req_wait_times, bool runs_realtime,
                      uint64_t less_margin, uint64_t more_margin) {
  for (osal_uint64_t req_wait_time_nsecs : req_wait_times) {
    uint64_t run_time_nsecs = measure_timer(req_wait_time_nsecs);

    if (verbose) {
      printf("runs realtime: %s \n", runs_realtime ? "yes" : "no");
      printf("requested time: %lu nsec, actual wait: %lu nsec\n",
             (ulong)req_wait_time_nsecs, (ulong)run_time_nsecs);
    }

    EXPECT_GE(run_time_nsecs, req_wait_time_nsecs + less_margin)
        << "measured time has negative difference from requested time";
    if (check_latency) {
      EXPECT_LE(run_time_nsecs, req_wait_time_nsecs + more_margin)
          << "Timer tardiness exceeds tolerance";
    }
  }
}

TEST(TimerExpired, SaneSingleThreaded) {
  const bool runs_realtime = is_realtime();

  // we allow no negative time tifference,
  // but a positive time difference of 10 μs when
  // running with real-time scheduling, and
  // 100 μs otherwise.
  const int64_t TIMER_TOLERANCE_LESS_NS = 0;
  const int64_t TIMER_TOLERANCE_MORE_NS = runs_realtime ? 10000 : 100000;

  vector<osal_uint64_t> req_wait_times = {500000000, 50000000, 5000000,
                                          500000,    50000,    5000};

  check_wait_times(req_wait_times, runs_realtime, TIMER_TOLERANCE_LESS_NS,
                   TIMER_TOLERANCE_MORE_NS);
}

/* set up to run timer waits with shuffled times
   in multiple threads */

typedef struct {
  vector<osal_uint64_t> wait_times;
  bool runs_realtime;
  int64_t less_margin;
  int64_t more_margin;
} wait_params_t;

void *check_wait_times_mt(void *vparams) {
  wait_params_t *p_params = (wait_params_t *)vparams;
  wait_params_t params = *p_params;
  check_wait_times(params.wait_times, params.runs_realtime, params.less_margin,
                   params.more_margin);
  return nullptr;
}

TEST(TimerExpired, SaneMultiThreaded) {
  const bool runs_realtime = is_realtime();

  // we allow no negative time tifference,
  // but a positive time difference of 10 μs when
  // running with real-time scheduling, and
  // 100 μs otherwise.
  const int64_t TIMER_TOLERANCE_LESS_NS = 0;
  const int64_t TIMER_TOLERANCE_MORE_NS = runs_realtime ? 50000 : 100000;
  // const int64_t TIMER_TOLERANCE_MORE_NS = 100;

  const int N_THREADS = 10;
  pthread_t thread_ids[N_THREADS];

  vector<osal_uint64_t> req_wait_times = {
      500000000, 200000000, 100000000, 50000000, 20000000, 10000000,
      5000000,   2000000,   1000000,   500000,   200000,   100000,
      50000,     20000,     10000,     5000,     2000,     1000};

  // because pthread_start passes only a single
  // argument pointer, we store parameters in
  // a struct.
  vector<wait_params_t> thread_params_vec;

  // randomize wait times and parameters
  // We do this ahead and keep each result
  // as a copy because we need to keep the
  // data constant while the threads
  // are running.
  for (int i = 0; i < N_THREADS; i++) {
    // create random-shuffled copy of wait times
    const int seed_value = i;
    vector<osal_uint64_t> shuffled_times =
        shuffle_vector(req_wait_times, seed_value);
    // store parameters
    const wait_params_t params = {shuffled_times, runs_realtime,
                                  TIMER_TOLERANCE_LESS_NS,
                                  TIMER_TOLERANCE_MORE_NS};
    thread_params_vec.push_back(params);
  }
  for (int i = 0; i < N_THREADS; i++) {
    if (verbose) {
      printf("starting thread %i\n", i);
    }
    pthread_create(/*thread*/ &(thread_ids[i]),
                   /*pthread_attr*/ nullptr,
                   /* start_routine */ check_wait_times_mt,
                   /* arg */ (void *)&(thread_params_vec[i]));
  }
  for (int i = 0; i < N_THREADS; i++) {
    if (verbose) {
      printf("joining thread %i\n", i);
    }
    pthread_join(/*thread*/ thread_ids[i],
                 /*retval*/ nullptr);
  }
}

uint64_t measure_sleep(long req_wait_time_nsecs) {
  struct timespec tp1;
  struct timespec tp2;
  clock_gettime(CLOCK_MONOTONIC, &tp1);

  osal_sleep(req_wait_time_nsecs);
  clock_gettime(CLOCK_MONOTONIC, &tp2);

  // get measured system time difference
  uint64_t time_diff_nsecs =
      ((tp2.tv_sec - tp1.tv_sec) * 1000000000 + (tp2.tv_nsec - tp1.tv_nsec));

  return time_diff_nsecs;
}

void check_sleep_times(vector<osal_uint64_t> &req_wait_times,
                       bool runs_realtime, uint64_t less_margin,
                       uint64_t more_margin) {
  for (osal_uint64_t req_wait_time_nsecs : req_wait_times) {
    uint64_t run_time_nsecs = measure_sleep(req_wait_time_nsecs);

    if (verbose) {
      printf("runs realtime: %s \n", runs_realtime ? "yes" : "no");
      printf("requested time: %lu nsec, actual wait: %lu nsec\n",
             (ulong)req_wait_time_nsecs, (ulong)run_time_nsecs);
    }

    EXPECT_GE(run_time_nsecs, req_wait_time_nsecs + less_margin)
        << "measured time has negative difference from requested time";
    if (check_latency) {
      EXPECT_LE(run_time_nsecs, req_wait_time_nsecs + more_margin)
          << "Timer tardiness exceeds tolerance";
    }
  }
}

TEST(TimerSleep, SaneSingleThreaded) {
  const bool runs_realtime = is_realtime();

  // we allow no negative time tifference,
  // but a positive time difference of 10 μs when
  // running with real-time scheduling, and
  // 100 μs otherwise.
  const int64_t TIMER_TOLERANCE_LESS_NS = 0;
  const int64_t TIMER_TOLERANCE_MORE_NS = runs_realtime ? 100000 : 150000;

  vector<osal_uint64_t> req_wait_times = {500000000, 50000000, 5000000,
                                          500000,    50000,    5000};

  check_sleep_times(req_wait_times, runs_realtime, TIMER_TOLERANCE_LESS_NS,
                    TIMER_TOLERANCE_MORE_NS);
}

/* now, we do the same check again, but
   in multiple parallel threas.
*/

typedef struct {
  vector<osal_uint64_t> wait_times;
  bool runs_realtime;
  int64_t less_margin;
  int64_t more_margin;
} sleep_params_t;

void *check_sleep_times_mt(void *vparams) {
  sleep_params_t *p_params = (sleep_params_t *)vparams;
  sleep_params_t params = *p_params;
  check_sleep_times(params.wait_times, params.runs_realtime, params.less_margin,
                    params.more_margin);
  return nullptr;
}

TEST(TimerSleep, SaneMultiThreaded) {
  const bool runs_realtime = is_realtime();

  // we allow no negative time tifference,
  // but a positive time difference of 10 μs when
  // running with real-time scheduling, and
  // 100 μs otherwise.
  const int64_t TIMER_TOLERANCE_LESS_NS = 0;
  const int64_t TIMER_TOLERANCE_MORE_NS = runs_realtime ? 100000 : 200000;
  // const int64_t TIMER_TOLERANCE_MORE_NS = 100;

  const int N_THREADS = 8;
  pthread_t thread_ids[N_THREADS];

  vector<osal_uint64_t> req_wait_times = {
      500000000, 200000000, 100000000, 50000000, 20000000, 10000000,
      5000000,   2000000,   1000000,   500000,   200000,   100000,
      50000,     20000,     10000,     5000,     2000,     1000};

  // because pthread_start passes only a single
  // argument pointer, we store parameters in
  // a struct.
  vector<sleep_params_t> thread_params_vec;

  // randomize wait times and parameters
  // We do this ahead and keep each result
  // as a copy because we need to keep the
  // data constant while the threads
  // are running.
  for (int i = 0; i < N_THREADS; i++) {
    // create random-shuffled copy of wait times
    const int seed_value = i;
    vector<osal_uint64_t> shuffled_times =
        shuffle_vector(req_wait_times, seed_value);
    // store parameters
    const sleep_params_t params = {shuffled_times, runs_realtime,
                                   TIMER_TOLERANCE_LESS_NS,
                                   TIMER_TOLERANCE_MORE_NS};
    thread_params_vec.push_back(params);
  }
  for (int i = 0; i < N_THREADS; i++) {
    if (verbose) {
      printf("starting thread %i\n", i);
    }
    pthread_create(/*thread*/ &(thread_ids[i]),
                   /*pthread_attr*/ nullptr,
                   /* start_routine */ check_sleep_times_mt,
                   /* arg */ (void *)&(thread_params_vec[i]));
  }
  for (int i = 0; i < N_THREADS; i++) {
    if (verbose) {
      printf("joining thread %i\n", i);
    }
    pthread_join(/*thread*/ thread_ids[i],
                 /*retval*/ nullptr);
  }
}

/*
  check for the osal_sleep_until() function.
*/

uint64_t measure_sleep_until(long req_wait_time_nsecs) {
  struct timespec tp1;
  struct timespec tp2;
  osal_timer_t timer;
  osal_timer_t timer_until;

  osal_timer_gettime(&timer);
  clock_gettime(CLOCK_MONOTONIC, &tp1);

  osal_timer_add_nsec(&timer, req_wait_time_nsecs, &timer_until);

  osal_sleep_until(&timer_until);
  clock_gettime(CLOCK_MONOTONIC, &tp2);

  // get measured system time difference
  uint64_t time_diff_nsecs =
      ((tp2.tv_sec - tp1.tv_sec) * 1000000000 + (tp2.tv_nsec - tp1.tv_nsec));

  return time_diff_nsecs;
}

void check_sleep_until(vector<osal_uint64_t> &req_wait_times,
                       bool runs_realtime, uint64_t less_margin,
                       uint64_t more_margin) {
  for (osal_uint64_t req_wait_time_nsecs : req_wait_times) {
    uint64_t run_time_nsecs = measure_sleep_until(req_wait_time_nsecs);

    if (verbose) {
      printf("runs realtime: %s \n", runs_realtime ? "yes" : "no");
      printf("requested time: %lu nsec, actual wait: %lu nsec\n",
             (ulong)req_wait_time_nsecs, (ulong)run_time_nsecs);
    }

    EXPECT_GE(run_time_nsecs, req_wait_time_nsecs + less_margin)
        << "measured time has negative difference from requested time";
    if (check_latency) {
      EXPECT_LE(run_time_nsecs, req_wait_time_nsecs + more_margin)
          << "Timer tardiness exceeds tolerance";
    }
  }
}

TEST(TimerSleepUntil, SaneSingleThreaded) {
  const bool runs_realtime = is_realtime();

  // we allow no negative time tifference,
  // but a positive time difference of 10 μs when
  // running with real-time scheduling, and
  // 100 μs otherwise.
  const int64_t TIMER_TOLERANCE_LESS_NS = 0;
  const int64_t TIMER_TOLERANCE_MORE_NS = runs_realtime ? 100000 : 150000;

  vector<osal_uint64_t> req_wait_times = {500000000, 50000000, 5000000,
                                          500000,    50000,    5000};

  check_sleep_until(req_wait_times, runs_realtime, TIMER_TOLERANCE_LESS_NS,
                    TIMER_TOLERANCE_MORE_NS);
}

typedef struct {
  vector<osal_uint64_t> wait_times;
  bool runs_realtime;
  int64_t less_margin;
  int64_t more_margin;
} sleep_until_params_t;

void *check_sleep_until_mt(void *vparams) {
  sleep_params_t *p_params = (sleep_params_t *)vparams;
  sleep_params_t params = *p_params;
  check_sleep_until(params.wait_times, params.runs_realtime, params.less_margin,
                    params.more_margin);
  return nullptr;
}

TEST(TimerSleepUntil, SaneMultiThreaded) {
  const bool runs_realtime = is_realtime();

  // we allow no negative time tifference,
  // but a positive time difference of 10 μs when
  // running with real-time scheduling, and
  // 100 μs otherwise.
  const int64_t TIMER_TOLERANCE_LESS_NS = 0;
  const int64_t TIMER_TOLERANCE_MORE_NS = runs_realtime ? 100000 : 200000;
  // const int64_t TIMER_TOLERANCE_MORE_NS = 100;

  const int N_THREADS = 8;
  pthread_t thread_ids[N_THREADS];

  vector<osal_uint64_t> req_wait_times = {
      500000000, 200000000, 100000000, 50000000, 20000000, 10000000,
      5000000,   2000000,   1000000,   500000,   200000,   100000,
      50000,     20000,     10000,     5000,     2000,     1000};

  // because pthread_start passes only a single
  // argument pointer, we store parameters in
  // a struct.
  vector<sleep_until_params_t> thread_params_vec;

  // randomize wait times and parameters
  // We do this ahead and keep each result
  // as a copy because we need to keep the
  // data constant while the threads
  // are running.
  for (int i = 0; i < N_THREADS; i++) {
    // create random-shuffled copy of wait times
    const int seed_value = i;
    vector<osal_uint64_t> shuffled_times =
        shuffle_vector(req_wait_times, seed_value);
    // store parameters
    const sleep_until_params_t params = {shuffled_times, runs_realtime,
                                         TIMER_TOLERANCE_LESS_NS,
                                         TIMER_TOLERANCE_MORE_NS};
    thread_params_vec.push_back(params);
  }
  for (int i = 0; i < N_THREADS; i++) {
    if (verbose) {
      printf("starting thread %i\n", i);
    }
    pthread_create(/*thread*/ &(thread_ids[i]),
                   /*pthread_attr*/ nullptr,
                   /* start_routine */ check_sleep_until_mt,
                   /* arg */ (void *)&(thread_params_vec[i]));
  }
  for (int i = 0; i < N_THREADS; i++) {
    if (verbose) {
      printf("joining thread %i\n", i);
    }
    pthread_join(/*thread*/ thread_ids[i],
                 /*retval*/ nullptr);
  }
}

}  // namespace test_timer

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  if (getenv("VERBOSE")) {
    test_timer::verbose = 1;
  }
  if (getenv("CHECK_LATENCY")) {
    test_timer::check_latency = 1;
  }
  // try to lock memory
  errno = 0;
  if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
    perror("test_timer: could not lock memory");
  }

  return RUN_ALL_TESTS();
}
