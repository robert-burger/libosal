#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <cassert>
#include <vector>
#include "gtest/gtest.h"
#include "libosal/osal.h"
#include "libosal/timer.h"
#include "test_utils.h"

namespace test_semaphore {

static int verbose = 0;
static int check_latency = 0;
using std::min;

using testutils::is_realtime;
using testutils::wait_nanoseconds;

namespace test_single_reader {
/* the following two tests runs two threads, a sender
   and a receiver, each of which references a common
   semaphore, and each of which use a shared counter.

   Each thread loops. In each iteration, the receiver may pause a
   random time. The sender registers the time and posts to the
   semaphore, and the reveiver waits to receive it, and upon
   receival registers that time.  Also, the sender writes a random
   number to shared memory and stores it in a private array, and the
   receiver stores the value in another array.

   The rationale of the test is that if the semaphore would not
   ensure ordering, the receive times would be before post times.
   Further, the stored random numbers need to match, if the
   tested semaphore in fact prevents race conditions
   (it needs to order memory access to be consistent).
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
  bool was_read;  // flag to signal finished iteration
  pthread_mutex_t wasread_mutex;
  pthread_cond_t wasread_cond;  // condition variable for signaling read

  struct timespec startwait_times[LOOPCOUNT];
  struct timespec read_times[LOOPCOUNT];
  unsigned long read_values[LOOPCOUNT];
} thread_param_t;

void *test_semaphore(void *p_params) {
  // max time for random wait
  const int MAX_WAIT_TIME_NS = 10000;

  assert(p_params != nullptr);
  // keep in mind that params is necessarily shared here,
  // differently from some other test code.
  thread_param_t *params = ((thread_param_t *)p_params);
  // initialize PRNG
  srand(1);
  osal_retval_t orv = {};
  int rv = {};
  for (uint i = 0; i < LOOPCOUNT; i++) {
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

// first case: receiver is already waiting,
// and will be blocked in wait() when post() happens.

TEST(Semaphore, DirectWait) {
  const uint64_t MAX_LAG_REALTIME_NSEC = 70000;
  const uint64_t MAX_LAG_BATCH_NSEC = 250000;

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
  params.wait_before_read = true;
  params.was_read = false;

  // osal_semaphore_attr_t attr = OSAL_SEMAPHORE_ATTR__PROCESS_SHARED;
  orv = osal_semaphore_init(&params.sema, nullptr, 0);
  ASSERT_EQ(orv, OSAL_OK) << "osal_semaphore_init() failed";

  srand(1);
  rv = pthread_create(/*thread*/ &(thread_id),
                      /*pthread_attr*/ nullptr,
                      /* start_routine */ test_semaphore,
                      /* arg */ (void *)&params);
  ASSERT_EQ(rv, 0) << "pthread_create() failed";

  for (uint i = 0; i < LOOPCOUNT; i++) {
    // generate and send random value
    unsigned long val = rand();
    params.value = val;
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
        printf("waiting maximally until %lu sec epoch\n", wait_time.tv_sec);
      }
      wait_time.tv_nsec = 0;
      rv = pthread_cond_timedwait(&params.wasread_cond, &params.wasread_mutex,
                                  &wait_time);
      ASSERT_EQ(rv, 0) << "pthread_cond_[timed]wait() failed";
    }
    params.was_read = false;
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

  orv = osal_semaphore_destroy(&params.sema);
  ASSERT_EQ(orv, OSAL_OK) << "osal_semaphore_destroy() failed";

  // Now, we have the send times in send_times[],
  // and the read times in params.read_times[]
  // Equally, we have the values that were
  // written to the shared memory in send_values[],
  // and the received values in parms.read_values[].

  // The values need to match, and the read time
  // must never be smaller than the send time,
  // otherwise the semaphore would be broken.

  for (ulong i = 0; i < LOOPCOUNT; i++) {
    EXPECT_EQ(send_values[i], params.read_values[i])
        << "sent and received values do not match";
  }

  int64_t max_lag =
      (is_realtime() ? MAX_LAG_REALTIME_NSEC : MAX_LAG_BATCH_NSEC);

  // We do a sanity check for the times -
  // Again, this is not meant as a check of real-time
  // performace, but just whether the implementation
  // is sane.
  // We require:
  // 1. receive times must never be before send times
  // 2. the time difference should not be too large

  for (ulong i = 0; i < LOOPCOUNT; i++) {
    timespec tp1 = send_times[i];
    timespec tp2 = params.read_times[i];
    //  cast here because the difference can be
    // negative, and tv_sec is unsigned.
    int64_t time_diff_nsecs =
        ((((int64_t)tp2.tv_sec) - ((int64_t)tp1.tv_sec)) * 1000000000 +
         (tp2.tv_nsec - tp1.tv_nsec));

    EXPECT_GE(time_diff_nsecs, 0) << "the read time was ahead of the send time";
    if (check_latency) {
      EXPECT_LT(time_diff_nsecs, max_lag)
          << "the time difference between wait() and send() was too large";
    }
  }
}

// second case: receiver sleeps for a random time,
// and might not have to block when wait()
// is called. This requires that such a delay
// does not cause the event to be lost.

TEST(Semaphore, RandomizedDelay) {
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
  params.wait_before_read = true;
  params.was_read = false;

  // osal_semaphore_attr_t attr = OSAL_SEMAPHORE_ATTR__PROCESS_SHARED;
  orv = osal_semaphore_init(&params.sema, nullptr, 0);
  ASSERT_EQ(orv, OSAL_OK) << "osal_semaphore_init() failed";

  srand(1);
  rv = pthread_create(/*thread*/ &(thread_id),
                      /*pthread_attr*/ nullptr,
                      /* start_routine */ test_semaphore,
                      /* arg */ (void *)&params);
  ASSERT_EQ(rv, 0) << "pthread_create() failed";

  for (uint i = 0; i < LOOPCOUNT; i++) {
    // generate and send random value
    unsigned long val = rand();
    params.value = val;
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
        printf("waiting maximally until %lu sec epoch\n", wait_time.tv_sec);
      }
      wait_time.tv_nsec = 0;
      rv = pthread_cond_timedwait(&params.wasread_cond, &params.wasread_mutex,
                                  &wait_time);
      ASSERT_EQ(rv, 0) << "pthread_cond_[timed]wait() failed";
    }
    params.was_read = false;
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

  orv = osal_semaphore_destroy(&params.sema);
  ASSERT_EQ(orv, OSAL_OK) << "osal_semaphore_destroy() failed";

  // Now, we have the send times in send_times[],
  // and the read times in params.read_times[]
  // Equally, we have the values that were
  // written to the shared memory in send_values[],
  // and the received values in parms.read_values[].

  // The values need to match, and the read time
  // must never be smaller than the send time,
  // otherwise the semaphore would be broken.

  for (ulong i = 0; i < LOOPCOUNT; i++) {
    EXPECT_EQ(send_values[i], params.read_values[i])
        << "sent and received values do not match";
  }

  int64_t max_lag =
      (is_realtime() ? MAX_LAG_REALTIME_NSEC : MAX_LAG_BATCH_NSEC);

  // We do a sanity check for the times -
  // Again, this is not meant as a check of real-time
  // performace, but just whether the implementation
  // is sane.
  // We require:
  // 1. receive times must never be before send times
  // 2. the time difference should not be too large

  for (ulong i = 0; i < LOOPCOUNT; i++) {
    timespec tp1 = send_times[i];
    timespec tp2 = params.startwait_times[i];
    timespec tp3 = params.read_times[i];
    //  cast here because the difference can be
    // negative, and tv_sec is unsigned.

    // time between send and read - must be positive
    int64_t time_diff31_nsecs =
        ((((int64_t)tp3.tv_sec) - ((int64_t)tp1.tv_sec)) * 1000000000 +
         (tp3.tv_nsec - tp1.tv_nsec));

    // time between start of wait and read - will be positive
    int64_t time_diff32_nsecs =
        ((((int64_t)tp3.tv_sec) - ((int64_t)tp2.tv_sec)) * 1000000000 +
         (tp3.tv_nsec - tp2.tv_nsec));

    // time between send and start of wait - can be negative
    int64_t time_diff21_nsecs =
        ((((int64_t)tp2.tv_sec) - ((int64_t)tp1.tv_sec)) * 1000000000 +
         (tp2.tv_nsec - tp1.tv_nsec));

    // reads before the matching send should not happen
    EXPECT_GE(time_diff31_nsecs, 0)
        << "the read time was ahead of the send time";

    if (check_latency) {
      // we discern whether wait() started earlier or later than send()
      if (time_diff21_nsecs < 0) {
        // wait started already before send, we look as before at
        // the difference between send and read time
        EXPECT_LT(time_diff31_nsecs, max_lag)
            << "the time difference between wait() and send() was too large";
      } else {
        // due to receiver sleeping, wait started after send, we
        // look at interval betweem wait and send
        EXPECT_LT(time_diff32_nsecs, max_lag)
            << "the time difference between wait() and start_wait() was too "
               "large";
      }
    }
  }
}
}  // namespace test_single_reader

/* The following tests test the semaphore with one
   sender and multiple receiver threads.

   Test 1: One sender, M receivers. The receivers count how
           many events they receive.

   Test 2: The sender holds a lock and sends multiple events
           at once, and the receives wait for that lock
           before they wait for the semaphore. Thus,
           they compete for the event.

   Test 3:  In addition to Test 2, the times are  compared.

*/

namespace multireader {
const int LOOPCOUNT2 = 10000;
const int NTHREADS = 50;

typedef struct {
  int thread_num;
  osal_semaphore_t *p_sema;
  std::atomic<bool> *pstop_flag;
  unsigned long count;
} thread_param_count_t;

void *test_semaphore_count(void *p_params) {
  assert(p_params != nullptr);
  // keep in mind that params is necessarily shared here,
  // differently from some other test code.
  thread_param_count_t *params = ((thread_param_count_t *)p_params);
  params->count = 0;
  osal_retval_t orv;
  while (true) {
    // note: if events are missed, this test will hang here
    orv = osal_semaphore_wait(params->p_sema);
    // note: this and the following are not assertions
    // because it does not work.... seems that
    // the ASSERT macros contain a return which does
    // not work for calles functions, while EXPECT_* does.
    EXPECT_EQ(orv, OSAL_OK) << "error in osal_semaphore_wait()";

    if (*params->pstop_flag) {
      if (verbose) {
        printf("thread %i: flag received, stopping at count = %lu\n",
               params->thread_num, params->count);
      }
      break;
    }
    // store the value passed from the sender
    params->count++;
  }

  return nullptr;
}

// this just sends a number of post() events to multiple
// receivers, which count the received events.
TEST(Semaphore, ParallelCount) {
  pthread_t thread_ids[NTHREADS];
  ;
  thread_param_count_t params[NTHREADS]; /* shared data protected by
                        semaphore and mutex */
  assert(NTHREADS > 0);
  assert(LOOPCOUNT2 > 0);
  osal_retval_t orv;
  osal_semaphore_t sema;
  std::atomic<bool> stop_flag(false);

  orv = osal_semaphore_init(&sema, nullptr, 0);
  ASSERT_EQ(orv, OSAL_OK) << "osal_semaphore_init() failed";

  int rv;
  for (int i = 0; i < NTHREADS; i++) {
    params[i].thread_num = i;
    params[i].p_sema = &sema;
    params[i].pstop_flag = &stop_flag;
    rv = pthread_create(/*thread*/ &(thread_ids[i]),
                        /*pthread_attr*/ nullptr,
                        /* start_routine */ test_semaphore_count,
                        /* arg */ (void *)&params[i]);
    ASSERT_EQ(rv, 0) << "pthread_create() failed";
  }
  printf("parallel sender: start OK\n");

  for (int i = 0; i < LOOPCOUNT2; i++) {
    orv = osal_semaphore_post(&sema);
    ASSERT_EQ(orv, OSAL_OK) << "osal_semaphore_post() failed";
  }
  sleep(1);
  // instruct threads to stop
  stop_flag = true;
  for (int i = 0; i < LOOPCOUNT2; i++) {
    orv = osal_semaphore_post(&sema);
    ASSERT_EQ(orv, OSAL_OK) << "osal_semaphore_post() failed";
  }
  printf("parallel sender: joining\n");

  long sum_count = 0;
  for (int i = 0; i < NTHREADS; i++) {
    rv = pthread_join(/*thread*/ thread_ids[i],
                      /*retval*/ nullptr);
    ASSERT_EQ(rv, 0) << "pthread_join() failed";
    sum_count += params[i].count;
  }
  orv = osal_semaphore_destroy(&sema);
  ASSERT_EQ(orv, OSAL_OK) << "osal_semaphore_destroy() failed";

  EXPECT_EQ(sum_count, LOOPCOUNT2) << "the count of events does not match";
}
}  // namespace multireader

namespace timedwait {
const int LOOPCOUNT3 = 1000;
const int NTHREADS = 10;
const int TIMEOUT_PERIOD_NSEC = 1000000;

typedef struct {
  int thread_num;
  osal_semaphore_t *p_sema;
  std::atomic<bool> *pstop_flag;
  std::atomic<unsigned long> count;
  std::atomic<unsigned long> timeout_count;
} thread_param_count_t;

void *test_semaphore_timedwait(void *p_params) {
  assert(p_params != nullptr);
  // keep in mind that params is necessarily shared here,
  // differently from some other test code.
  thread_param_count_t *params = ((thread_param_count_t *)p_params);
  params->count = 0;
  params->timeout_count = 0;
  osal_retval_t orv;
  osal_timer_t deadline_osal = {};  // this is an absolute time!!
  struct timespec deadline_posix = {};
  while (true) {
    int rv = clock_gettime(CLOCK_REALTIME, &deadline_posix);
    EXPECT_EQ(rv, 0) << "could not read realtime clock";

    deadline_osal.sec = deadline_posix.tv_sec;
    deadline_osal.nsec = deadline_posix.tv_nsec + TIMEOUT_PERIOD_NSEC;
    // normalize input
    while (deadline_osal.nsec > 1000000000) {
      deadline_osal.nsec -= 10000000000;
      deadline_osal.sec += 1;
    }
    // note: if stop events are missed, this test will hang here
    // this can happen if there is an error with the semaphore
    // implementation.
    orv = osal_semaphore_timedwait(params->p_sema, &deadline_osal);
    // note: this and the following are not assertions
    // because it does not work.... seems that
    // the ASSERT macros contain a return which does
    // not work for calles functions, while EXPECT_* does.
    if (*params->pstop_flag) {
      if (verbose) {
        unsigned long count = params->count;
        printf("thread %i: flag received, stopping at count = %lu\n",
               params->thread_num, count);
      }
      break;
    }

    if (orv == OSAL_ERR_TIMEOUT) {
      params->timeout_count++;
    } else {
      EXPECT_EQ(orv, OSAL_OK) << "error in osal_semaphore_wait()";

      // store the value passed from the sender
      params->count++;
    }
  }

  return nullptr;
}

// this just sends a number of post() events to multiple
// receivers, which count the received events.
TEST(Semaphore, TimedCount) {
  pthread_t thread_ids[NTHREADS];
  ;
  thread_param_count_t params[NTHREADS]; /* shared data protected by
                        semaphore and mutex */
  assert(NTHREADS > 0);
  assert(LOOPCOUNT3 > 0);
  osal_retval_t orv;
  osal_semaphore_t sema;
  std::atomic<bool> stop_flag(false);

  orv = osal_semaphore_init(&sema, nullptr, 0);
  ASSERT_EQ(orv, OSAL_OK) << "osal_semaphore_init() failed";

  int rv;
  for (int i = 0; i < NTHREADS; i++) {
    params[i].thread_num = i;
    params[i].p_sema = &sema;
    params[i].pstop_flag = &stop_flag;
    rv = pthread_create(/*thread*/ &(thread_ids[i]),
                        /*pthread_attr*/ nullptr,
                        /* start_routine */ test_semaphore_timedwait,
                        /* arg */ (void *)&params[i]);
    ASSERT_EQ(rv, 0) << "pthread_create() failed";
  }
  printf("parallel sender: start OK\n");

  srand(1);
  long sum_delays = 0;
  const int DELAY_UNIT = TIMEOUT_PERIOD_NSEC / NTHREADS;
  // the idea is as follows: with 1 delay units (1ms/N)
  // for each post(), the sender can, in the ideal case, exactly keep up
  // without the N receivers having repeated timeouts.
  // Each extra delayunit should cause on average
  // one extra timeout.
  // System latency can of course cause /more/ timeouts,
  // but we should never see fewer.
  for (int i = 0; i < LOOPCOUNT3; i++) {
    int tick = 1 /* DELAY_UNIT */;
    int extra_delay = rand() % 10;
    wait_nanoseconds(DELAY_UNIT * (tick + extra_delay));
    sum_delays += extra_delay;

    orv = osal_semaphore_post(&sema);
    ASSERT_EQ(orv, OSAL_OK) << "osal_semaphore_post() failed";
  }

  /* wait for threads to finish counting.  the following codes
     uses the arrangement that the counter params[i].count, which
     is compared to, is atomic.  We expect the counter to match
     LOOPCOUNT3 as long as the semaphore implementation does not
     lose events.
  */

  long sum_count = 0;
  /* note that the wait time can be SURPRISINGLY large */
  long max_wait_time = 10000000000;  /* 10 seconds */
  const long wait_period = 10000000; /* 10 ms */
  while (max_wait_time > 0) {
    sum_count = 0;
    for (int i = 0; i < NTHREADS; i++) {
      sum_count += params[i].count;
    }
    if (sum_count == LOOPCOUNT3) {
      break;  // all threads have finished
    }
    wait_nanoseconds(wait_period);
    max_wait_time -= min(max_wait_time, wait_period);
  }

  // now, instruct threads to stop
  stop_flag = true;
  for (int i = 0; i < NTHREADS; i++) {
    orv = osal_semaphore_post(&sema);
    ASSERT_EQ(orv, OSAL_OK) << "osal_semaphore_post() failed";
  }
  printf("parallel sender: joining\n");

  long sum_timeout_count = 0;
  for (int i = 0; i < NTHREADS; i++) {
    rv = pthread_join(/*thread*/ thread_ids[i],
                      /*retval*/ nullptr);
    ASSERT_EQ(rv, 0) << "pthread_join() failed";
    sum_timeout_count += params[i].timeout_count;
  }
  orv = osal_semaphore_destroy(&sema);
  ASSERT_EQ(orv, OSAL_OK) << "osal_semaphore_destroy() failed";
  printf(
      "test timeout_wait: %li delays introduced,"
      " %li timeouts observed\n",
      sum_delays, sum_timeout_count);

  EXPECT_EQ(sum_count, LOOPCOUNT3) << "the count of events does not match";

  /* we cannot assert for the number of timeouts here, because
     they can differ in both directions. */
}
}  // namespace timedwait

namespace trywait {
const int LOOPCOUNT4 = 1000;
const int NTHREADS = 10;
const int WAIT_PERIOD_NSEC = 1000000;

typedef struct {
  int thread_num;
  osal_semaphore_t *p_sema;
  std::atomic<bool> *pstop_flag;
  std::atomic<unsigned long> count;
  unsigned long wait_count;
} thread_param_try_t;

void *test_semaphore_trywait(void *p_params) {
  assert(p_params != nullptr);
  // keep in mind that params is necessarily shared here,
  // differently from some other test code.
  thread_param_try_t *params = ((thread_param_try_t *)p_params);
  params->count = 0;
  params->wait_count = 0;
  osal_retval_t orv;
  while (true) {
    // wait an interval which should normally catch a signal
    wait_nanoseconds(WAIT_PERIOD_NSEC);

    // note: if stop events are missed, this test will hang here
    // this can happen if there is an error with the semaphore
    // implementation.
    orv = osal_semaphore_trywait(params->p_sema);
    // note: this and the following are not assertions
    // because it does not work.... seems that
    // the ASSERT macros contain a return which does
    // not work for calles functions, while EXPECT_* does.
    if ((*params->pstop_flag) || (params->wait_count > 10000000)) {
      if (verbose) {
        unsigned long count = params->count;
        printf("thread %i: stopping at count = %lu\n", params->thread_num,
               count);
      }
      break;
    }

    if (orv == OSAL_ERR_BUSY) {
      params->wait_count++;

    } else {
      EXPECT_EQ(orv, OSAL_OK) << "error in osal_semaphore_trywait()";

      // store the value passed from the sender
      params->count++;
    }
  }

  return nullptr;
}

// this just sends a number of post() events to multiple
// receivers, which count the received events.
TEST(Semaphore, TryCount) {
  pthread_t thread_ids[NTHREADS];
  ;
  thread_param_try_t params[NTHREADS]; /* shared data protected by
                        semaphore and mutex */
  assert(NTHREADS > 0);
  assert(LOOPCOUNT4 > 0);
  osal_retval_t orv;
  osal_semaphore_t sema;
  std::atomic<bool> stop_flag(false);

  orv = osal_semaphore_init(&sema, nullptr, 0);
  ASSERT_EQ(orv, OSAL_OK) << "osal_semaphore_init() failed";

  int rv;
  for (int i = 0; i < NTHREADS; i++) {
    params[i].thread_num = i;
    params[i].p_sema = &sema;
    params[i].pstop_flag = &stop_flag;
    rv = pthread_create(/*thread*/ &(thread_ids[i]),
                        /*pthread_attr*/ nullptr,
                        /* start_routine */ test_semaphore_trywait,
                        /* arg */ (void *)&params[i]);
    ASSERT_EQ(rv, 0) << "pthread_create() failed";
  }
  printf("parallel sender: start OK\n");

  srand(1);
  long sum_delays = 0;
  const int DELAY_UNIT = WAIT_PERIOD_NSEC / NTHREADS;
  // the idea is as follows: with 1 delay units (1ms/N)
  // for each post(), the sender can, in the ideal case, exactly keep up
  // without the N receivers having repeated timeouts.
  // Each extra delayunit should cause on average
  // one extra timeout.
  // System latency can of course cause /more/ timeouts,
  // but we should never see fewer.
  for (int i = 0; i < LOOPCOUNT4; i++) {
    int tick = 1 /* DELAY_UNIT */;
    int extra_delay = rand() % 10;
    wait_nanoseconds(DELAY_UNIT * (tick + extra_delay));
    sum_delays += extra_delay;

    orv = osal_semaphore_post(&sema);
    ASSERT_EQ(orv, OSAL_OK) << "osal_semaphore_post() failed";
  }

  long sum_count = 0;
  /* note that the wait time can be SURPRISINGLY large */
  long max_wait_time = 10000000000; /* 10 seconds */
  const long wait_period = 1000000; /* 1 ms */
  while (max_wait_time > 0) {
    sum_count = 0;
    for (int i = 0; i < NTHREADS; i++) {
      sum_count += params[i].count;
    }
    if (sum_count == LOOPCOUNT4) {
      break;  // all threads have finished
    }
    wait_nanoseconds(wait_period);
    max_wait_time -= min(max_wait_time, wait_period);
  }

  // instruct threads to stop, by setting flag and waiting
  stop_flag = true;
  for (int i = 0; i < NTHREADS; i++) {
    orv = osal_semaphore_post(&sema);
    ASSERT_EQ(orv, OSAL_OK) << "osal_semaphore_post() failed";
  }
  printf("parallel sender: joining\n");

  long sum_wait_count = 0;
  for (int i = 0; i < NTHREADS; i++) {
    rv = pthread_join(/*thread*/ thread_ids[i],
                      /*retval*/ nullptr);
    ASSERT_EQ(rv, 0) << "pthread_join() failed";
    sum_wait_count += params[i].wait_count;
  }
  orv = osal_semaphore_destroy(&sema);
  ASSERT_EQ(orv, OSAL_OK) << "osal_semaphore_destroy() failed";
  printf(
      "test timeout_wait: %li delays introduced,"
      " %li timeouts observed\n",
      sum_delays, sum_wait_count);

  EXPECT_EQ(sum_count, LOOPCOUNT4) << "the count of events does not match";
  EXPECT_GE(sum_wait_count, sum_delays) << "some timeouts were not detected";
}
}  // namespace trywait

}  // namespace test_semaphore

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  if (getenv("VERBOSE")) {
    test_semaphore::verbose = 1;
  }
  if (getenv("CHECK_LATENCY")) {
    test_semaphore::check_latency = 1;
  }
  // try to lock memory
  errno = 0;
  if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
    perror("test_semaphore: could not lock memory");
  }
  return RUN_ALL_TESTS();
}
