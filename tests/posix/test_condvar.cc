#include "libosal/condvar.h"
#include "libosal/osal.h"
#include "libosal/timer.h"
#include "test_utils.h"
#include "gtest/gtest.h"
#include <cassert>
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <vector>
namespace test_condvar {

static int verbose = 0;
static int check_latency = 0;
using std::min;

using testutils::is_realtime;
using testutils::pick_random_from_bits;
using testutils::popcount;
using testutils::wait_nanoseconds;

/* helper function which gets bitmask of unoccupied threads.
 */
int get_free_mask(std::atomic<ulong> *active_mask, int min_number,
                  int thread_number) {
  assert(thread_number > 0);
  assert(min_number <= thread_number);
  int out_free_mask;
  while (true) {
    out_free_mask = (~*active_mask) & ((1 << thread_number) - 1);
    if (popcount(out_free_mask) >= min_number) {
      return out_free_mask;
    } else {
      wait_nanoseconds(10000);
    }
  }
  return out_free_mask;
}

/* Test of condition variables.

   This test starts N=16 threads, which wait for
   the same condition variable. The data that it
   watches for is the number of the thread which is
   addressed, and a send time.

   A further shared item is an atomic bitmask
   which indicates which threads are free to receive
   a new notification. The sender selects a free
   thread, sends it a notification that uses
   the condition variable, and marks it as busy in
   the bit mask.

   On receiving the condition, a waiting thread
   increments a counter, evaluates the time difference
   between sending and receiving, and resets its
   bitmask entry.

   For finishing, the sender sets a stop flag,
   and broadcasts the condition change.
*/

namespace condvar_multithread {
const int LOOPCOUNT1 = 50000;
const int NTHREADS = 20;
const int NO_THREAD = -1;
// minimum number of free threads before
// the test proceeds to select a free thread
// (this is introduced in order to incrase
// randomness).
const int MIN_FREE_THREADS_PROCEED = 2;

/* Note: differently from some other tests, this struct
   is shared between all threads! */

typedef struct {
  osal_condvar_t condvar;
  osal_mutex_t mutex;
  std::atomic<unsigned long> active_mask;
  // begin of items protected by mutex
  unsigned long event_count[NTHREADS];
  bool stop;
  // end of items protected by mutex

  // piping for clean termination
  sem_t finished_sem;            // is signaled when a thread terminates
  std::atomic<int> thread_count; // count of active threads

} shared_t;

typedef struct {
  int thread_id;
  shared_t *p_shared;

} threadvar_t;

void *test_condvar_count(void *arg) {
  assert(arg != nullptr);
  // keep in mind that shared_objects is necessarily shared here,
  // differently from some other test code.
  threadvar_t thread_var = *((threadvar_t *)arg);
  const int thread_id = thread_var.thread_id;
  shared_t *p_shared_objects = thread_var.p_shared;

  osal_retval_t orv;
  int rv;
  while (true) {
    // note: if events are missed, this test will hang here
    osal_mutex_lock(&p_shared_objects->mutex);
    EXPECT_EQ(orv, OSAL_OK) << "error in receiver: osal_mutex_lock()";

    while (!((p_shared_objects->active_mask >> thread_id) & 1ul) &&
           (!p_shared_objects->stop)) {
      orv = osal_condvar_wait(&p_shared_objects->condvar,
                              &p_shared_objects->mutex);
      EXPECT_EQ(orv, OSAL_OK) << "error in receiver: osal_condvar_wait()";
    }

    if ((p_shared_objects->active_mask >> thread_id) & 1ul) {
      p_shared_objects->event_count[thread_id]++;
    }

    if (p_shared_objects->stop) {
      if (verbose) {
        printf("thread %i: stop signal received\n", thread_id);
      }
      orv = osal_mutex_unlock(&p_shared_objects->mutex);
      EXPECT_EQ(orv, OSAL_OK) << "error in receiver: osal_mutex_unlock()";
      break;
    }
    // increment event count, which is the "paylod" of the
    // transaction

    orv = osal_mutex_unlock(&p_shared_objects->mutex);
    EXPECT_EQ(orv, OSAL_OK) << "error in receiver: mutex_unlock failed()";

    // register that thread is ready to receive new
    // messages. This can be done outside of the lock
    // since this uses an atomic variable.
    ulong clear_mask = ~(1ul << thread_id);
    std::atomic_fetch_and(&p_shared_objects->active_mask, clear_mask);
  }

  // signal that thread is finished
  p_shared_objects->thread_count -= 1;
  rv = sem_post(&p_shared_objects->finished_sem);
  EXPECT_EQ(rv, 0) << "error when sending finished sem signal";

  return nullptr;
}

// this just sends a number of post() events to multiple
// receivers, which count the received events.
TEST(CondvarFunction, ParallelMasked) {
  shared_t shared_objects; /* shared data protected by
                              condvar and mutex */

  threadvar_t thread_vars[NTHREADS];

  static_assert(NTHREADS <= CHAR_BIT * sizeof(shared_objects.active_mask));
  shared_objects.active_mask = 0;
  shared_objects.thread_count = 0;

  shared_objects.stop = false;

  assert(NTHREADS > 0);
  assert(LOOPCOUNT1 > 0);
  osal_retval_t orv;
  int rv;

  pthread_t thread_ids[NTHREADS];
  unsigned long event_count[NTHREADS];

  rv = sem_init(&shared_objects.finished_sem, 0, 0);
  ASSERT_EQ(rv, 0) << "creating pthreads semaphore failed!";

  orv = osal_condvar_init(&shared_objects.condvar, nullptr);
  ASSERT_EQ(orv, OSAL_OK) << "osal_condvar_init() failed";

  orv = osal_mutex_init(&shared_objects.mutex, nullptr);
  ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_init() failed";

  // initialize count of active threads
  shared_objects.thread_count = NTHREADS;
  memset(event_count, 0, sizeof(event_count));
  memset(shared_objects.event_count, 0, sizeof(shared_objects.event_count));
  // start threads
  for (int i = 0; i < NTHREADS; i++) {
    thread_vars[i].p_shared = &shared_objects;
    thread_vars[i].thread_id = i;

    rv = pthread_create(/*thread*/ &(thread_ids[i]),
                        /*pthread_attr*/ nullptr,
                        /* start_routine */ test_condvar_count,
                        /* arg */ (void *)&thread_vars[i]);
    ASSERT_EQ(rv, 0) << "pthread_create() failed";
  }

  if (verbose) {
    printf("parallel sender: start OK\n");
  }

  srand(1);
  for (int i = 0; i < LOOPCOUNT1; i++) {
    // get bitmask of idle threads; if there
    // are few, wait a bit.
    // This can be done outside of the lock
    // since the mask is an atomic variable.
    // (what needs to be synchronized is the change
    // from 0 to 1 for a bit.)
    ulong free_mask = get_free_mask(&shared_objects.active_mask,
                                    MIN_FREE_THREADS_PROCEED, NTHREADS);
    // pick randomly some idle thread
    int selected_thread = pick_random_from_bits(free_mask);
    // register that thread as busy by flipping its bit
    orv = osal_mutex_lock(&shared_objects.mutex);
    std::atomic_fetch_or(&shared_objects.active_mask, (1ul << selected_thread));
    ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_lock() failed";

    orv = osal_condvar_broadcast(&shared_objects.condvar);
    ASSERT_EQ(orv, OSAL_OK) << "osal_condvar_broadcast() failed";

    // track thread-local count of sent messages for that thread
    event_count[selected_thread]++;

    orv = osal_mutex_unlock(&shared_objects.mutex);
    ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_unlock() failed";
  }

  orv = osal_mutex_lock(&shared_objects.mutex);
  ASSERT_EQ(orv, OSAL_OK) << "stopping: osal_mutex_lock() failed";
  // instruct threads to stop
  shared_objects.stop = true;
  // broadcast change
  orv = osal_condvar_broadcast(&shared_objects.condvar);
  ASSERT_EQ(orv, OSAL_OK) << "stopping: osal_condvar_broadcast() failed";
  orv = osal_mutex_unlock(&shared_objects.mutex);
  ASSERT_EQ(orv, OSAL_OK) << "stopping: osal_mutex_unlock() failed";

  struct timespec max_time = {time(nullptr) + 10, 0};

  while (shared_objects.thread_count > 0) {
    rv = sem_timedwait(&shared_objects.finished_sem, &max_time);
    EXPECT_EQ(rv, 0) << "wait for termination failed";
    if (rv) {
      printf("wait for termination of threads timed out\n");
      break;
    }
  }
  if (verbose) {
    printf("parallel sender: joining\n");
  }
  for (int i = 0; i < NTHREADS; i++) {
    rv = pthread_join(/*thread*/ thread_ids[i],
                      /*retval*/ nullptr);
    EXPECT_EQ(rv, 0) << "pthread_join() failed";
  }

  rv = sem_destroy(&shared_objects.finished_sem);
  EXPECT_EQ(rv, 0) << "could not destroy mutex";

  orv = osal_condvar_destroy(&shared_objects.condvar);
  EXPECT_EQ(orv, OSAL_OK) << "osal_condvar_destroy() failed";

  orv = osal_mutex_destroy(&shared_objects.mutex);
  ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_destroy() failed";

  // checking resulting counts

  if (verbose) {
    for (int i = 0; i < NTHREADS; i++) {
      printf("count for thread %i: local = %lu, thread = %lu\n", i,
             event_count[i], shared_objects.event_count[i]);
    }
  }
  for (int i = 0; i < NTHREADS; i++) {
    EXPECT_EQ(event_count[i], shared_objects.event_count[i])
        << "the count of events does not match";
  }
}
} // namespace condvar_multithread

/*
 This test is very similar to the multithread test,
 but with the difference that each thread
 does not get a broadcast signaling
 but a single one.
 */
namespace condvar_single {
const int LOOPCOUNT3 = 50000;
const int NTHREADS = 20;
const int NO_THREAD = -1;
// minimum number of free threads before
// the test proceeds to select a free thread
// (this is introduced in order to incrase
// randomness).
const int MIN_FREE_THREADS_PROCEED = 2;

/* Note: differently from some other tests, this struct
   is shared between all threads! */

typedef struct {
  osal_condvar_t condvar[NTHREADS];
  osal_mutex_t mutex[NTHREADS];

  // begin of items protected by mutex
  unsigned long event_count[NTHREADS];
  // end of items protected by mutex
  std::atomic<unsigned long> active_mask;

  std::atomic<bool> stop;

  // indicator for actual termination
  std::atomic<int> thread_count; // count of active threads

} shared_t;

typedef struct {
  int thread_id;
  std::atomic<bool> has_stopped;
  shared_t *p_shared;

} threadvar_separate_t;

void *test_condvar_separate(void *arg) {
  assert(arg != nullptr);
  // keep in mind that shared_objects is necessarily shared here,
  // differently from some other test code.
  threadvar_separate_t *thread_var = ((threadvar_separate_t *)arg);
  const int thread_id = thread_var->thread_id;
  shared_t *p_shared_objects = thread_var->p_shared;

  osal_retval_t orv;

  p_shared_objects->thread_count++; // signal start of thread
  while (true) {
    // note: if events are missed, this test will hang here
    osal_mutex_lock(&p_shared_objects->mutex[thread_id]);
    EXPECT_EQ(orv, OSAL_OK) << "error in receiver: osal_mutex_lock()";

    orv = osal_condvar_wait(&p_shared_objects->condvar[thread_id],
                            &p_shared_objects->mutex[thread_id]);
    EXPECT_EQ(orv, OSAL_OK) << "error in receiver: osal_condvar_wait()";

    if ((p_shared_objects->active_mask >> thread_id) & 1ul) {
      p_shared_objects->event_count[thread_id]++;
    }

    // using p_shared_objects->stop is safe here because it
    // is an atomic variable.
    if (p_shared_objects->stop) {
      if (verbose) {
        printf("thread %i: stop signal received\n", thread_id);
      }
      orv = osal_mutex_unlock(&p_shared_objects->mutex[thread_id]);
      EXPECT_EQ(orv, OSAL_OK) << "error in receiver: osal_mutex_unlock()";
      break;
    }
    // increment event count, which is the "paylod" of the
    // transaction

    // register that thread is ready to receive new
    // messages. This can be done outside of the lock
    // since this uses an atomic variable.
    ulong clear_mask = ~(1ul << thread_id);
    std::atomic_fetch_and(&p_shared_objects->active_mask, clear_mask);

    orv = osal_mutex_unlock(&p_shared_objects->mutex[thread_id]);
    EXPECT_EQ(orv, OSAL_OK) << "error in receiver: mutex_unlock failed()";
  }

  // signal that thread is finished
  if (verbose) {
    printf("final count for thread %i: final =  %lu\n", thread_id,
           p_shared_objects->event_count[thread_id]);
  }

  p_shared_objects->thread_count -= 1;
  thread_var->has_stopped = true;

  return nullptr;
}

// this just sends a number of post() events to multiple
// receivers, which count the received events.
// The difference to the other tests is that each
// receiver holds an own lock and condition variable,
// and is notified by post() instead of broadcast.

TEST(CondvarFunction, ParallelSingleNotification) {
  shared_t shared_objects; /* shared data protected by
                              condvar and mutex */

  threadvar_separate_t thread_vars[NTHREADS];

  shared_objects.thread_count = 0;

  shared_objects.stop = false;

  assert(NTHREADS > 0);
  assert(LOOPCOUNT3 > 0);
  static_assert(NTHREADS <= CHAR_BIT * sizeof(shared_objects.active_mask));
  osal_retval_t orv;
  int rv;

  pthread_t thread_ids[NTHREADS];
  unsigned long event_count[NTHREADS];

  ulong wait_count = 0;

  shared_objects.active_mask = 0;

  // initialize count of active threads
  shared_objects.thread_count = 0;
  memset(event_count, 0, sizeof(event_count));
  memset(shared_objects.event_count, 0, sizeof(shared_objects.event_count));
  // start threads
  for (int i = 0; i < NTHREADS; i++) {
    orv = osal_condvar_init(&shared_objects.condvar[i], nullptr);
    ASSERT_EQ(orv, OSAL_OK) << "osal_condvar_init() failed";

    orv = osal_mutex_init(&shared_objects.mutex[i], nullptr);
    ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_init() failed";

    thread_vars[i].p_shared = &shared_objects;
    thread_vars[i].thread_id = i;
    thread_vars[i].has_stopped = false;

    rv = pthread_create(/*thread*/ &(thread_ids[i]),
                        /*pthread_attr*/ nullptr,
                        /* start_routine */ test_condvar_separate,
                        /* arg */ (void *)&thread_vars[i]);
    ASSERT_EQ(rv, 0) << "pthread_create() failed";

    // wait for that thread to start, so that no
    // events are missed later
    while (shared_objects.thread_count <= i) {
      wait_nanoseconds(1000000); // one millisecond
      wait_count++;
      ASSERT_LE(wait_count, 5000ul) << "wait for thread start failed";
    }
  }

  if (verbose) {
    printf("parallel sender: start OK\n");
  }

  srand(1);
  for (int i = 0; i < LOOPCOUNT3; i++) {
    // pick randomly some idle thread
    ulong free_mask = get_free_mask(&shared_objects.active_mask,
                                    MIN_FREE_THREADS_PROCEED, NTHREADS);
    // pick randomly some non-busy thread
    int selected_thread = pick_random_from_bits(free_mask);
    // register that thread as busy by flipping its bit
    orv = osal_mutex_lock(&shared_objects.mutex[selected_thread]);
    ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_lock() failed";

    std::atomic_fetch_or(&shared_objects.active_mask, (1ul << selected_thread));
    // here, send a single notification to that thread

    orv = osal_condvar_signal(&shared_objects.condvar[selected_thread]);
    ASSERT_EQ(orv, OSAL_OK) << "osal_condvar_broadcast() failed";

    // track thread-local count of sent messages for that thread
    event_count[selected_thread]++;

    orv = osal_mutex_unlock(&shared_objects.mutex[selected_thread]);
    ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_unlock() failed";
  }

  // instruct threads to stop, by setting an atomic flag,
  // and sending a condition broadcast
  shared_objects.stop = true;
  while (shared_objects.thread_count > 0) {
    for (int i = 0; i < NTHREADS; i++) {
      if (!thread_vars[i].has_stopped) {
        orv = osal_mutex_lock(&shared_objects.mutex[i]);
        ASSERT_EQ(orv, OSAL_OK) << "stopping: osal_mutex_lock() failed";

        orv = osal_condvar_signal(&shared_objects.condvar[i]);
        ASSERT_EQ(orv, OSAL_OK) << "stopping: osal_condvar_broadcast() failed";
        orv = osal_mutex_unlock(&shared_objects.mutex[i]);
        ASSERT_EQ(orv, OSAL_OK) << "stopping: osal_mutex_unlock() failed";
      }
    }

    if (shared_objects.thread_count > 0) {
      wait_nanoseconds(1000000); // one millisecond
      wait_count++;
      ASSERT_LE(wait_count, 10000ul) << "stopping: wait for termination failed";
    }
  }
  if (verbose) {
    printf("parallel sender: joining\n");
  }
  for (int i = 0; i < NTHREADS; i++) {
    rv = pthread_join(/*thread*/ thread_ids[i],
                      /*retval*/ nullptr);
    EXPECT_EQ(rv, 0) << "pthread_join() failed";
  }

  for (int i = 0; i < NTHREADS; i++) {
    orv = osal_condvar_destroy(&shared_objects.condvar[i]);
    EXPECT_EQ(orv, OSAL_OK) << "osal_condvar_destroy() failed";

    orv = osal_mutex_destroy(&shared_objects.mutex[i]);
    ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_destroy() failed";
  }

  // checking resulting counts

  if (verbose) {
    for (int i = 0; i < NTHREADS; i++) {
      printf("count for thread %i: local = %lu, thread = %lu\n", i,
             event_count[i], shared_objects.event_count[i]);
    }
  }
  for (int i = 0; i < NTHREADS; i++) {
    EXPECT_EQ(event_count[i], shared_objects.event_count[i])
        << "the count of events does not match";
  }
}
} // namespace condvar_single

/* the test below repeat these above with the timed wait variant */

namespace condvar_timedwait {
const int LOOPCOUNT2 = 50000;
const int NTHREADS = 20;
const int NO_THREAD = -1;
const uint64_t MAX_WAIT_TIME_NSEC = 500000;
const uint64_t MAX_WAIT_LATENCY = 1000000;

// minimum number of free threads before
// the test proceeds to select a free thread
// (this is introduced in order to incrase
// randomness).
const int MIN_FREE_THREADS_PROCEED = 2;

/* Note: differently from some other tests, this struct
   is shared between all threads! */

typedef struct {
  osal_condvar_t condvar;
  osal_mutex_t mutex;
  std::atomic<unsigned long> active_mask;
  // begin of items protected by mutex
  unsigned long event_count[NTHREADS];

  struct timespec post_time[NTHREADS];
  unsigned long max_wait_time[NTHREADS];
  bool stop;
  // end of items protected by mutex

  // piping for clean termination
  sem_t finished_sem;            // is signaled when a thread terminates
  std::atomic<int> thread_count; // count of active threads

} shared_wait_t;

typedef struct {
  int thread_id;
  shared_wait_t *p_shared;

} threadvar_wait_t;

void *test_condvar_wait(void *arg) {
  assert(arg != nullptr);
  // keep in mind that shared_objects is necessarily shared here,
  // differently from some other test code.
  threadvar_wait_t thread_var = *((threadvar_wait_t *)arg);
  const int thread_id = thread_var.thread_id;
  shared_wait_t *p_shared_objects = thread_var.p_shared;

  osal_retval_t orv;
  int rv;
  osal_timer_t max_wait_time;
  timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  now.tv_nsec += MAX_WAIT_TIME_NSEC;
  if (now.tv_nsec > 1000000000) {
    now.tv_nsec -= 1000000000;
    now.tv_sec += 1;
  }
  max_wait_time.sec = now.tv_sec;
  max_wait_time.nsec = now.tv_nsec;

  while (true) {
    // note: if events are missed, this test will hang here
    orv = osal_mutex_lock(&p_shared_objects->mutex);
    EXPECT_EQ(orv, OSAL_OK) << "error in receiver: osal_mutex_lock()";

    while (!((p_shared_objects->active_mask >> thread_id) & 1ul) &&
           (!p_shared_objects->stop)) {
      orv = osal_condvar_timedwait(&p_shared_objects->condvar,
                                   &p_shared_objects->mutex, &max_wait_time);
      if (orv == OSAL_ERR_TIMEOUT) {
        continue;
      }
      EXPECT_EQ(orv, OSAL_OK) << "error in receiver: osal_condvar_wait()";
    }

    if ((p_shared_objects->active_mask >> thread_id) & 1ul) {
      p_shared_objects->event_count[thread_id]++;
    }

    struct timespec receive_time;
    clock_gettime(CLOCK_MONOTONIC, &receive_time);
    struct timespec &post_time = p_shared_objects->post_time[thread_id];
    ulong post_nsec = post_time.tv_sec * 1000000000 + post_time.tv_nsec;
    ulong receive_nsec =
        receive_time.tv_sec * 1000000000 + receive_time.tv_nsec;

    // collect maximum wait time
    ulong wait_time = receive_nsec - post_nsec;
    if (wait_time > p_shared_objects->max_wait_time[thread_id]) {
      p_shared_objects->max_wait_time[thread_id] = wait_time;
    }

    if (p_shared_objects->stop) {
      if (verbose) {
        printf("thread %i: stop signal received\n", thread_id);
      }
      orv = osal_mutex_unlock(&p_shared_objects->mutex);
      EXPECT_EQ(orv, OSAL_OK) << "error in receiver: osal_mutex_unlock()";
      break;
    }
    // increment event count, which is the "paylod" of the
    // transaction

    orv = osal_mutex_unlock(&p_shared_objects->mutex);
    EXPECT_EQ(orv, OSAL_OK) << "error in receiver: mutex_unlock failed()";

    // register that thread is ready to receive new
    // messages. This can be done outside of the lock
    // since this uses an atomic variable.
    ulong clear_mask = ~(1ul << thread_id);
    std::atomic_fetch_and(&p_shared_objects->active_mask, clear_mask);
  }

  // signal that thread is finished
  p_shared_objects->thread_count -= 1;
  rv = sem_post(&p_shared_objects->finished_sem);
  EXPECT_EQ(rv, 0) << "error when sending finished sem signal";

  return nullptr;
}

// this just sends a number of post() events to multiple
// receivers, which count the received events.
TEST(CondvarFunction, ParallelWait) {
  shared_wait_t shared_objects; /* shared data protected by
                              condvar and mutex */

  threadvar_wait_t thread_vars[NTHREADS];

  static_assert(NTHREADS <= CHAR_BIT * sizeof(shared_objects.active_mask));
  shared_objects.active_mask = 0;
  shared_objects.thread_count = 0;

  shared_objects.stop = false;

  assert(NTHREADS > 0);
  assert(LOOPCOUNT2 > 0);
  osal_retval_t orv;
  int rv;

  pthread_t thread_ids[NTHREADS];
  unsigned long event_count[NTHREADS];

  rv = sem_init(&shared_objects.finished_sem, 0, 0);
  ASSERT_EQ(rv, 0) << "creating pthreads semaphore failed!";

  orv = osal_condvar_init(&shared_objects.condvar, nullptr);
  ASSERT_EQ(orv, OSAL_OK) << "osal_condvar_init() failed";

  orv = osal_mutex_init(&shared_objects.mutex, nullptr);
  ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_init() failed";

  // initialize count of active threads
  shared_objects.thread_count = NTHREADS;
  memset(event_count, 0, sizeof(event_count));
  memset(shared_objects.event_count, 0, sizeof(shared_objects.event_count));
  memset(shared_objects.max_wait_time, 0, sizeof(shared_objects.max_wait_time));
  // start threads
  for (int i = 0; i < NTHREADS; i++) {
    thread_vars[i].p_shared = &shared_objects;
    thread_vars[i].thread_id = i;

    rv = pthread_create(/*thread*/ &(thread_ids[i]),
                        /*pthread_attr*/ nullptr,
                        /* start_routine */ test_condvar_wait,
                        /* arg */ (void *)&thread_vars[i]);
    ASSERT_EQ(rv, 0) << "pthread_create() failed";
  }

  if (verbose) {
    printf("parallel sender: start OK\n");
  }

  srand(1);
  for (int i = 0; i < LOOPCOUNT2; i++) {
    // get bitmask of idle threads; if there
    // are few, wait a bit.
    // This can be done outside of the lock
    // since the mask is an atomic variable.
    // (what needs to be synchronized is the change
    // from 0 to 1 for a bit.)
    ulong free_mask = get_free_mask(&shared_objects.active_mask,
                                    MIN_FREE_THREADS_PROCEED, NTHREADS);
    // pick randomly some idle thread
    int selected_thread = pick_random_from_bits(free_mask);
    // register that thread as busy by flipping its bit
    orv = osal_mutex_lock(&shared_objects.mutex);
    std::atomic_fetch_or(&shared_objects.active_mask, (1ul << selected_thread));
    ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_lock() failed";

    // register time of sending event
    clock_gettime(CLOCK_MONOTONIC, &shared_objects.post_time[selected_thread]);

    orv = osal_condvar_broadcast(&shared_objects.condvar);
    ASSERT_EQ(orv, OSAL_OK) << "osal_condvar_broadcast() failed";

    // track thread-local count of sent messages for that thread
    event_count[selected_thread]++;

    orv = osal_mutex_unlock(&shared_objects.mutex);
    ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_unlock() failed";
  }

  orv = osal_mutex_lock(&shared_objects.mutex);
  ASSERT_EQ(orv, OSAL_OK) << "stopping: osal_mutex_lock() failed";
  // instruct threads to stop
  shared_objects.stop = true;
  sleep(1);
  // broadcast change
  orv = osal_condvar_broadcast(&shared_objects.condvar);
  ASSERT_EQ(orv, OSAL_OK) << "stopping: osal_condvar_broadcast() failed";
  orv = osal_mutex_unlock(&shared_objects.mutex);
  ASSERT_EQ(orv, OSAL_OK) << "stopping: osal_mutex_unlock() failed";

  struct timespec max_time = {time(nullptr) + 10, 0};

  while (shared_objects.thread_count > 0) {
    rv = sem_timedwait(&shared_objects.finished_sem, &max_time);
    EXPECT_EQ(rv, 0) << "wait for termination failed";
    if (rv) {
      printf("wait for termination of threads timed out\n");
      break;
    }
  }
  if (verbose) {
    printf("parallel sender: joining\n");
  }
  for (int i = 0; i < NTHREADS; i++) {
    rv = pthread_join(/*thread*/ thread_ids[i],
                      /*retval*/ nullptr);
    EXPECT_EQ(rv, 0) << "pthread_join() failed";
  }

  rv = sem_destroy(&shared_objects.finished_sem);
  EXPECT_EQ(rv, 0) << "could not destroy mutex";

  orv = osal_condvar_destroy(&shared_objects.condvar);
  EXPECT_EQ(orv, OSAL_OK) << "osal_condvar_destroy() failed";

  orv = osal_mutex_destroy(&shared_objects.mutex);
  ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_destroy() failed";

  // checking resulting counts

  if (verbose) {
    for (int i = 0; i < NTHREADS; i++) {
      printf("count for thread %i: local = %lu, thread = %lu\n", i,
             event_count[i], shared_objects.event_count[i]);
    }
    for (int i = 0; i < NTHREADS; i++) {
      printf("max wait time for thread %i:  %lu\n", i,
             shared_objects.max_wait_time[i]);
    }
  }

  if (check_latency) {
    for (int i = 0; i < NTHREADS; i++) {
      EXPECT_LE(shared_objects.max_wait_time[i], MAX_WAIT_LATENCY);
    }
  }

  for (int i = 0; i < NTHREADS; i++) {
    EXPECT_EQ(event_count[i], shared_objects.event_count[i])
        << "the count of events does not match";
  }
}
} // namespace condvar_timedwait

} // namespace test_condvar

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  if (getenv("VERBOSE")) {
    test_condvar::verbose = 1;
  }
  if (getenv("CHECK_LATENCY")) {
    test_condvar::check_latency = 1;
  }
  // try to lock memory
  errno = 0;
  if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
    perror("test_condvar: [info] could not lock memory");
  }
  return RUN_ALL_TESTS();
}
