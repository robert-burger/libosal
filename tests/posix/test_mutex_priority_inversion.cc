#include "libosal/osal.h"
#include "libosal/mutex.h"
#include "libosal/task.h"
#include "test_utils.h"
#include "gtest/gtest.h"
#include <atomic>
#include <pthread.h>
#include <vector>

namespace test_mutex {

extern int verbose;

namespace test_priority_inversion {

/*

  Structure of test:

  - task L, M, H, with low/medium/high priority
  - the test requires real-time scheduling, each task
    must only run when no task with higher priority
    is runnable
  - a mutexes M, shared between task L and task H.
  - two atomic flags L_started and H_waiting, and
    a third atomic flag L_finished signals completion.
    Being atomic, these  do not need to be protected by a lock.
  - task L holds mutex M
  - task H sets flag H_waiting which asks task L to compute something, which
    takes three seconds.
  - during its computation, task L holds mutex M
  - task L then sets flag L_started to indicate work has started.
    This advances H, which tries to acquire Mutex M,
    bumping up the priority of L.
  - After three second of computation, Task L sets flag L_finished and
    waits for task M and task H to exit, which they do on
    detecting L_finished.
  - When task M observes flag x, it starts a ten-second
    CPU-intensive computation.
  - task H records the time between its request to task L
    and the completion signalled by flag L_finished, and exits.
  - when the priority inversion works, H will have to
    wait 3 seconds, when it fails, 13 seconds. This is
    the test criterion.

*/

using testutils::wait_nanoseconds;

typedef struct {
  osal_mutex_t mutexM;
  std::atomic<bool> flag_L_started;
  std::atomic<bool> flag_H_waiting;
  std::atomic<bool> flag_L_finished;
  uint32_t hash = {};
  double time_delta = 0.0;
} shared_t;

void *run_H(void *p_params) {

  osal_retval_t orv = {};
  shared_t *p_shared = (shared_t *)p_params;

  osal_task_sched_priority_t base_prio = 0;
  orv = osal_task_get_priority(nullptr, &base_prio);
  if (orv) {
    printf("orv = %i in osal_task_get_priority (H) in %s : %i \n", orv,
           __FILE__, __LINE__);
  }

  if (verbose) {
    printf("Task H: got priority = %i\n", (int)base_prio);
  }

  sleep(1); // wait for everything to start up

  // request work from task L
  p_shared->flag_H_waiting = true;

  if (orv) {
    printf("orv = %i in osal_mutex_unlock (B) in %s : %i \n", orv, __FILE__,
           __LINE__);
  }
  assert(orv == 0);

  // wait for start of L before trying to get lock

  while (!p_shared->flag_L_started) {
    wait_nanoseconds(500000000);
  }

  double start_time = time(nullptr);
  double stop_time = 0.0;

  // now, Mutex B should be locked by L.
  // We wait for the lock to be released.
  orv = osal_mutex_lock(&p_shared->mutexM);
  if (orv) {
    printf("orv = %i in osal_mutex_lock (B) in %s : %i \n", orv, __FILE__,
           __LINE__);
  }
  assert(orv == 0);

  bool finished = false;
  while (!finished) {

    // check for completion of work by task L
    if (p_shared->flag_L_finished) {
      finished = true;
      stop_time = time(nullptr);
      p_shared->time_delta = stop_time - start_time;
    }
    if (!finished) {
      wait_nanoseconds(1000000); // 1 ms
    }
  }
  // we are finished and release the high-priority mutex
  orv = osal_mutex_unlock(&p_shared->mutexM);
  if (orv) {
    printf("orv = %i in osal_mutex_unlock (B) in %s : %i \n", orv, __FILE__,
           __LINE__);
  }
  assert(orv == 0);

  osal_task_delete();
  return nullptr;
}

void *run_M(void *p_params) {
  using testutils::combine_hashes;
  using testutils::hash_u32;

  osal_retval_t orv = {};
  shared_t *p_shared = (shared_t *)p_params;

  osal_task_sched_priority_t base_prio = 0;
  orv = osal_task_get_priority(nullptr, &base_prio);
  if (orv) {
    printf("orv = %i in osal_task_get_priority (M) in %s : %i \n", orv,
           __FILE__, __LINE__);
  }

  if (verbose) {
    printf("Task M: got priority = %i\n", (int)base_prio);
  }

  // wait for task L to start
  while (!(p_shared->flag_L_started)) {
    wait_nanoseconds(1000000); // 1 ms
  }

  wait_nanoseconds(100000000);

  time_t start_time = time(nullptr);
  time_t stop_time = start_time + 10.0;

  // do something that keeps the CPU busy
  uint32_t hash = 1;
  while ((time(nullptr) < stop_time) && (!(p_shared->flag_L_finished))) {
    for (int i = 0; i < 100000; i++) {
      hash = combine_hashes(hash, hash_u32(start_time & 0xFFFFFFFF));
    }
  }

  // ensure side effect (to avoid optimizing out the loop)
  p_shared->hash = hash;

  osal_task_delete();
  return nullptr;
}

void *run_L(shared_t *p_shared) {

  osal_retval_t orv;

  osal_task_sched_priority_t base_prio = 0;
  orv = osal_task_get_priority(nullptr, &base_prio);
  if (orv) {
    printf("orv = %i in osal_task_get_priority (L) in %s : %i \n", orv,
           __FILE__, __LINE__);
  }

  if (verbose) {
    printf("Task L: got priority = %i\n", (int)base_prio);
  }

  orv = osal_mutex_lock(&p_shared->mutexM);
  if (orv) {
    printf("orv = %i in osal_mutex_lock (B) in %s : %i \n", orv, __FILE__,
           __LINE__);
  }
  assert(orv == 0);
  // wait for H to be waiting for mutex B
  while (!p_shared->flag_H_waiting) {
    wait_nanoseconds(1000000);
  }

  // notify M, "inviting" interference from it
  p_shared->flag_L_started = true;

  for (int i = 0; i < 3000; i++) {
    // run for 1 second, while holding mutex B
    wait_nanoseconds(1000000); // wait 1 ms
  }

  /* set "finished" flag to signal termination */
  p_shared->flag_L_finished = true;

  orv = osal_mutex_unlock(&p_shared->mutexM);
  if (orv) {
    printf("orv = %i in osal_mutex_unlock (B) in %s : %i \n", orv, __FILE__,
           __LINE__);
  }
  assert(orv == 0);

  /* not deleting task: this runs in the main task */
  return nullptr;
}

TEST(MutexFunction, TestNoPriorityInheritance) {
  shared_t shared = {};

  osal_task_t task_H;
  osal_task_t task_M;

  osal_mutex_attr_t attr = OSAL_MUTEX_ATTR__ROBUST;

  osal_task_attr_t task_attr = {};
  task_attr.policy = OSAL_SCHED_POLICY_FIFO;
  task_attr.priority = 0;
  task_attr.affinity = 1;

  osal_retval_t orv = {};

  orv = osal_mutex_init(&shared.mutexM, &attr);
  ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_init() B failed";

  shared.flag_L_started = false;
  shared.flag_H_waiting = false;
  shared.flag_L_finished = false;

  task_attr.priority = 3;
  strcpy(task_attr.task_name, "Task_H");

  orv = osal_task_create(/*thread*/ &(task_H),
                         /*osal_task_attr*/ &task_attr,
                         /* start_routine */ run_H,
                         /* arg */ (void *)&shared);
  ASSERT_EQ(orv, OSAL_OK) << "osal_task_create() H failed";

  task_attr.priority = 2;
  strcpy(task_attr.task_name, "Task_M");
  orv = osal_task_create(/*thread*/ &(task_M),
                         /*osal_task_attr*/ &task_attr,
                         /* start_routine */ run_M,
                         /* arg */ (void *)&shared);

  ASSERT_EQ(orv, OSAL_OK) << "osal_task_create() M failed";

  orv = osal_task_set_policy(nullptr, OSAL_SCHED_POLICY_FIFO);
  if (orv == OSAL_ERR_PERMISSION_DENIED) {
      printf("Warning: osal_task_set_policy permission denied "
              "on that build host! Skipping TestNoPriorityInheritance!\n");
      return;
  }
  ASSERT_EQ(orv, OSAL_OK) << "osal_task_set_policy() L failed";

  orv = osal_task_set_priority(nullptr, 1);
  if (orv != 0) {
    printf("Warning: osal_task_set_priority() L failed "
           "- consider running under \"chrt -f 1 ...\"\n");
  }

  orv = osal_task_set_affinity(nullptr, 1u);
  if (orv != 0) {
    printf("Warning: osal_task_set_affinity() L failed "
           "- consider running under \"chrt -f 1 ...\"\n");
  }

  run_L(&shared);

  osal_task_retval_t trv = 0;

  orv = osal_task_join(&task_M, &trv);
  ASSERT_EQ(orv, OSAL_OK) << "osal_task_join M failed";

  orv = osal_task_join(&task_H, &trv);
  ASSERT_EQ(orv, OSAL_OK) << "osal_task_join H failed";

  orv = osal_mutex_destroy(&shared.mutexM);
  ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_destroy B failed";

  // here, because  task does not L inherit priority from H, time_delta should
  // be 13 sec, otherwise 3 sec, because L blocks M in this case.
  if (verbose) {
    printf("priority inheritance test: time delta = %f\n", shared.time_delta);
  }
  EXPECT_GT(shared.time_delta, 5.0) << ("no priority inversion provoked");
}

TEST(MutexFunction, TestPriorityInheritance) {
  shared_t shared = {};

  osal_task_t task_H;
  osal_task_t task_M;

  osal_mutex_attr_t attr = OSAL_MUTEX_ATTR__PROTOCOL__INHERIT;

  osal_task_attr_t task_attr = {};
  task_attr.policy = OSAL_SCHED_POLICY_FIFO;
  task_attr.priority = 0;
  task_attr.affinity = 1;

  osal_retval_t orv = {};

  orv = osal_mutex_init(&shared.mutexM, &attr);
  ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_init() B failed";

  shared.flag_L_started = false;
  shared.flag_H_waiting = false;
  shared.flag_L_finished = false;

  task_attr.priority = 3;
  orv = osal_task_create(/*thread*/ &(task_H),
                         /*osal_task_attr*/ &task_attr,
                         /* start_routine */ run_H,
                         /* arg */ (void *)&shared);
  ASSERT_EQ(orv, OSAL_OK) << "osal_task_create() H failed";

  task_attr.priority = 2;
  orv = osal_task_create(/*thread*/ &(task_M),
                         /*osal_task_attr*/ &task_attr,
                         /* start_routine */ run_M,
                         /* arg */ (void *)&shared);

  ASSERT_EQ(orv, OSAL_OK) << "osal_task_create() M failed";

  orv = osal_task_set_priority(nullptr, 1);
  if (orv != 0) {
    printf("Warning: osal_task_set_priority() L failed "
           "- consider running under \"chrt -f 1 ...\"\n");
  }

  orv = osal_task_set_affinity(nullptr, 1u);
  if (orv != 0) {
    printf("Warning: osal_task_set_affinity() L failed "
           "- consider running under \"chrt -f 1 ...\"\n");
  }

  orv = osal_task_set_policy(nullptr, OSAL_SCHED_POLICY_FIFO);
  ASSERT_EQ(orv, OSAL_OK) << "osal_task_set_policy() L failed";

  run_L(&shared);

  osal_task_retval_t trv = 0;

  orv = osal_task_join(&task_M, &trv);
  ASSERT_EQ(orv, OSAL_OK) << "osal_task_join M failed";

  orv = osal_task_join(&task_H, &trv);
  ASSERT_EQ(orv, OSAL_OK) << "osal_task_join H failed";

  orv = osal_mutex_destroy(&shared.mutexM);
  ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_destroy B failed";

  // here, if task L inherits priority from H, time_delta should be 3
  // sec (L blocking M), otherwise 13 sec, because M blocks L in this case.
  if (verbose) {
    printf("priority inheritance test: time delta = %f\n", shared.time_delta);
  }
  EXPECT_LT(shared.time_delta, 5.0) << (" priority adjustment failed");
}

TEST(MutexFunc, TestPriorityCeiling) {
  shared_t shared = {};

  osal_task_t task_H;
  osal_task_t task_M;

  osal_mutex_attr_t attr = (OSAL_MUTEX_ATTR__PROTOCOL__PROTECT |
                            (3 << OSAL_MUTEX_ATTR__PRIOCEILING__SHIFT));

  osal_task_attr_t task_attr = {};
  task_attr.policy = OSAL_SCHED_POLICY_FIFO;
  task_attr.priority = 0;
  task_attr.affinity = 1;

  osal_retval_t orv = {};

  orv = osal_mutex_init(&shared.mutexM, &attr);
  ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_init() B failed";

  shared.flag_L_started = false;
  shared.flag_H_waiting = false;
  shared.flag_L_finished = false;

  task_attr.priority = 3;
  orv = osal_task_create(/*thread*/ &(task_H),
                         /*osal_task_attr*/ &task_attr,
                         /* start_routine */ run_H,
                         /* arg */ (void *)&shared);
  ASSERT_EQ(orv, OSAL_OK) << "osal_task_create() H failed";

  task_attr.priority = 2;
  orv = osal_task_create(/*thread*/ &(task_M),
                         /*osal_task_attr*/ &task_attr,
                         /* start_routine */ run_M,
                         /* arg */ (void *)&shared);

  ASSERT_EQ(orv, OSAL_OK) << "osal_task_create() M failed";

  orv = osal_task_set_priority(nullptr, 1);
  if (orv != 0) {
    printf("Warning: osal_task_set_priority() L failed "
           "- consider running under \"chrt -f 1 ...\"\n");
  }

  orv = osal_task_set_affinity(nullptr, 1u);
  if (orv != 0) {
    printf("Warning: osal_task_set_affinity() L failed "
           "- consider running under \"chrt -f 1 ...\"\n");
  }

  orv = osal_task_set_policy(nullptr, OSAL_SCHED_POLICY_FIFO);
  ASSERT_EQ(orv, OSAL_OK) << "osal_task_set_policy() L failed";

  run_L(&shared);

  osal_task_retval_t trv = 0;

  orv = osal_task_join(&task_M, &trv);
  ASSERT_EQ(orv, OSAL_OK) << "osal_task_join M failed";

  orv = osal_task_join(&task_H, &trv);
  ASSERT_EQ(orv, OSAL_OK) << "osal_task_join H failed";

  orv = osal_mutex_destroy(&shared.mutexM);
  ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_destroy B failed";

  // here, if task L inherits priority from H, time_delta should be 3
  // sec (L blocking M), otherwise 13 sec, because M blocks L in this case.
  if (verbose) {
    printf("priority inheritance test: time delta = %f\n", shared.time_delta);
  }
  EXPECT_LT(shared.time_delta, 5.0) << (" priority adjustment failed");
}

TEST(MutexDetect, TestPriorityError) {
  /* test whether a priority that is too high is detected
     in the priority ceiling protocol (PROTOCOL_PROTECT). */
  osal_mutex_t mutexE;
  osal_mutex_attr_t attr = (OSAL_MUTEX_ATTR__PROTOCOL__PROTECT |
                            (2 << OSAL_MUTEX_ATTR__PRIOCEILING__SHIFT));

  osal_retval_t orv = {};

  orv = osal_mutex_init(&mutexE, &attr);
  ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_init() B failed";

  orv = osal_task_set_priority(nullptr, 3);
  if (orv != 0) {
    printf("Warning: osal_task_set_priority() L failed "
           "- consider running under \"chrt -f 1 ...\"\n");
  }

  orv = osal_task_set_policy(nullptr, OSAL_SCHED_POLICY_FIFO);
  ASSERT_EQ(orv, OSAL_OK) << "osal_task_set_policy() E failed";

  orv = osal_mutex_lock(&mutexE);
  EXPECT_EQ(orv, OSAL_ERR_INVALID_PARAM) << "osal_mutex_lock() E failed";

  if (!orv) {
    orv = osal_mutex_unlock(&mutexE);
  }

  orv = osal_mutex_destroy(&mutexE);
  ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_destroy E failed";
}

} // namespace test_priority_inversion

} // namespace test_mutex
