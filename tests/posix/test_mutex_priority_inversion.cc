#include "gtest/gtest.h"
#include <pthread.h>
#include <vector>

#include "libosal/osal.h"
#include "test_utils.h"

namespace test_priority_inversion {

/*

  Structure of test:

  - task L, M, H, with low/medium/high priority
  - the test requires real-time scheduling, each task
    must only run when no task with higher priority
    is runnable
  - two mutexes A and B, A shared between task L and task M,
    mutex B between task H and task L.
  - two flags x (protected by mutex A) and y (protected by B)
  - a third flag z signals completion
  - task L holds mutex A, and task M polls for it
  - task H sets flag y which asks task L to compute something, which
    takes one second, during which task L holds mutex B
  - task L then sets flag x and periodically acquires and
    releases mutex A which could be captured y task M.

    Using mutex A and mutex B, task L sets flag z and
    waits for task M and task H to exit.

  - When task M acquires observes flag x, it keeps mutex A
    for 10 seconds straight, and exits. If z it set,
    it exits.
  - task H records the time between its request to task L
    and the completion signalled by flag z, and exits.
  - when the priority inversion works, H will have to
    wait 1 second, when it fails, 10 seconds

*/

using testutils::wait_nanoseconds;

typedef struct {
  osal_mutex_t mutexA;
  osal_mutex_t mutexB;
  bool flag_x = {};
  bool flag_y = {};
  bool flag_z = {};
  int work_count = 0;
  double time_delta = 0.0;
} shared_t;

void *run_H(void *p_params) {

  osal_retval_t orv = {};
  shared_t *p_shared = (shared_t *)p_params;

  sleep(1); // wait for everything to start up

  orv = osal_mutex_lock(&p_shared->mutexB);
  if (orv) {
    printf("orv = %i in osal_mutex_lock (B) in %s : %i \n", orv, __FILE__,
           __LINE__);
  }
  assert(orv == 0);
  // request work from task L
  p_shared->flag_y = true;
  orv = osal_mutex_unlock(&p_shared->mutexB);
  if (orv) {
    printf("orv = %i in osal_mutex_unlock (B) in %s : %i \n", orv, __FILE__,
           __LINE__);
  }
  assert(orv == 0);

  double start_time = time(nullptr);
  double stop_time = 0.0;

  bool finished = false;
  while (!finished) {
    orv = osal_mutex_lock(&p_shared->mutexB);
    if (orv) {
      printf("orv = %i in osal_mutex_lock (B) in %s : %i \n", orv, __FILE__,
             __LINE__);
    }
    assert(orv == 0);

    // check for completion of work by task L
    if (p_shared->flag_z) {
      finished = true;
      stop_time = time(nullptr);
      p_shared->time_delta = stop_time - start_time;
    }
    orv = osal_mutex_unlock(&p_shared->mutexB);
    if (orv) {
      printf("orv = %i in osal_mutex_unlock (B) in %s : %i \n", orv, __FILE__,
             __LINE__);
    }
    assert(orv == 0);
    if (!finished) {
      wait_nanoseconds(1000000);
    }
  }

  osal_task_delete();
  return nullptr;
}

void *run_M(void *p_params) {

  osal_retval_t orv = {};
  shared_t *p_shared = (shared_t *)p_params;

  osal_task_sched_priority_t base_prio = 0;
  orv = osal_task_get_priority(nullptr, &base_prio);
  if (orv) {
    printf("orv = %i in osal_task_get_priority (M) in %s : %i \n", orv,
           __FILE__, __LINE__);
  }

  printf("Task M: got base priority = %i\n", (int)base_prio);

  bool finished = false;
  while (!finished) {
    orv = osal_mutex_lock(&p_shared->mutexA);
    if (orv) {
      printf("orv = %i in osal_mutex_lock (A) in %s : %i \n", orv, __FILE__,
             __LINE__);
    }
    assert(orv == 0);

    // wait for task L's flag indicating that it is working
    if (p_shared->flag_x && (!p_shared->flag_z)) {
      // block task L for 10 seconds, to provoke priority inversion

      osal_task_sched_priority_t prio = 0;
      orv = osal_task_get_priority(nullptr, &prio);
      if (orv) {
        printf("orv = %i in osal_task_get_priority (M) in %s : %i \n", orv,
               __FILE__, __LINE__);
      }

      printf("Task M: got actual priority = %i\n", (int)prio);

      if (prio <= base_prio) {
        sleep(10);
      }
      finished = true;
    } else if (p_shared->flag_z) {
      // we have slept past L's execution and can exit already
      finished = true;
    }
    orv = osal_mutex_unlock(&p_shared->mutexA);
    if (orv) {
      printf("orv = %i in osal_mutex_unlock (A) in %s : %i \n", orv, __FILE__,
             __LINE__);
    }
    assert(orv == 0);
    if (!finished) {
      wait_nanoseconds(1000000);
      continue;
    }
  }

  osal_task_delete();
  return nullptr;
}

void *run_L(shared_t *p_shared) {

  osal_retval_t orv;
  while (true) {

    orv = osal_mutex_lock(&p_shared->mutexB);
    if (orv) {
      printf("orv = %i in osal_mutex_lock (B) in %s : %i \n", orv, __FILE__,
             __LINE__);
    }
    assert(orv == 0);
    bool do_work = p_shared->flag_y;
    if (do_work) {
      // keep mutex B here to inherit task H's priority
      orv = osal_mutex_lock(&p_shared->mutexA);
      if (orv) {
        printf("orv = %i in osal_mutex_lock (A) in %s : %i \n", orv, __FILE__,
               __LINE__);
      }
      assert(orv == 0);
      // notify M, "inviting" interference from it
      p_shared->flag_x = true;
      orv = osal_mutex_unlock(&p_shared->mutexA);
      if (orv) {
        printf("orv = %i in osal_mutex_unlock (A) in %s : %i \n", orv, __FILE__,
               __LINE__);
      }
      assert(orv == 0);

      for (int i = 0; i < 1000; i++) {
        // work a bit, while holding mutex B
        wait_nanoseconds(1000000); // wait 1 ms

        /* yield to task M, if running. If M
           is actually running, this will result
           in a one-time ten-second delay. */
        orv = osal_mutex_lock(&p_shared->mutexA);
        if (orv) {
          printf("orv = %i in osal_mutex_lock (A) in %s : %i \n", orv, __FILE__,
                 __LINE__);
        }
        assert(orv == 0);

        /* the assignment is just to assure some
           side-effect and avoid optimizing out the loop.
        */
        p_shared->work_count = i;
        orv = osal_mutex_unlock(&p_shared->mutexA);
        if (orv) {
          printf("orv = %i in osal_mutex_unlock (A) in %s : %i \n", orv,
                 __FILE__, __LINE__);
        }
        assert(orv == 0);
      }

      /* set "finished" flag to signal termination - this
         requires to hold both mutexes */
      orv = osal_mutex_lock(&p_shared->mutexA);
      if (orv) {
        printf("orv = %i in osal_mutex_lock (A) in %s : %i \n", orv, __FILE__,
               __LINE__);
      }
      assert(orv == 0);

      p_shared->flag_z = true;
      orv = osal_mutex_unlock(&p_shared->mutexA);
      if (orv) {
        printf("orv = %i in osal_mutex_unlock (A) in %s : %i \n", orv, __FILE__,
               __LINE__);
      }
      assert(orv == 0);

      orv = osal_mutex_unlock(&p_shared->mutexB);
      if (orv) {
        printf("orv = %i in osal_mutex_unlock (B) in %s : %i \n", orv, __FILE__,
               __LINE__);
      }
      assert(orv == 0);

      break; // exit loop
    } else {
      orv = osal_mutex_unlock(&p_shared->mutexB);
      if (orv) {
        printf("orv = %i in osal_mutex_unlock (B) in %s : %i \n", orv, __FILE__,
               __LINE__);
      }
      assert(orv == 0);

      // stay in loop
    }
  }
  /* not deleting task: this runs in the main task */
  return nullptr;
}

TEST(MutexFunc, TestNoPriorityInheritance) {
  shared_t shared = {};

  osal_task_t task_H;
  osal_task_t task_M;

  osal_mutex_attr_t attr = OSAL_MUTEX_ATTR__ROBUST;
  osal_retval_t orv = {};

  orv = osal_mutex_init(&shared.mutexA, &attr);
  ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_init() A failed";

  orv = osal_mutex_init(&shared.mutexB, &attr);
  ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_init() B failed";

  shared.flag_x = false;
  shared.flag_y = false;
  shared.flag_z = false;

  orv = osal_task_create(/*thread*/ &(task_H),
                         /*osal_task_attr*/ nullptr,
                         /* start_routine */ run_H,
                         /* arg */ (void *)&shared);
  ASSERT_EQ(orv, OSAL_OK) << "osal_task_create() H failed";

  orv = osal_task_create(/*thread*/ &(task_M),
                         /*osal_task_attr*/ nullptr,
                         /* start_routine */ run_M,
                         /* arg */ (void *)&shared);

  ASSERT_EQ(orv, OSAL_OK) << "osal_task_create() M failed";

  run_L(&shared);

  osal_task_retval_t trv = 0;

  orv = osal_task_join(&task_M, &trv);
  ASSERT_EQ(orv, OSAL_OK) << "osal_task_join M failed";

  orv = osal_task_join(&task_H, &trv);
  ASSERT_EQ(orv, OSAL_OK) << "osal_task_join H failed";

  orv = osal_mutex_destroy(&shared.mutexA);
  ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_destroy A failed";

  orv = osal_mutex_destroy(&shared.mutexB);
  ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_destroy B failed";

  // here, if task L inherits priority from H, time_delta
  // should be 1 sec, otherwise 10 sec.
  printf("no priority inheritance test: time delta = %f\n", shared.time_delta);
  EXPECT_GT(shared.time_delta, 9.0) << (" priority adjustment failed");
}

TEST(MutexFunc, TestPriorityInheritance) {
  shared_t shared = {};

  osal_task_t task_H;
  osal_task_t task_M;

  osal_mutex_attr_t attr = OSAL_MUTEX_ATTR__PROTOCOL__INHERIT;

  osal_task_attr_t task_attr = {};
  task_attr.policy = OSAL_SCHED_POLICY_FIFO;
  task_attr.priority = 0;

  osal_retval_t orv = {};

  orv = osal_mutex_init(&shared.mutexA, &attr);
  ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_init() A failed";

  orv = osal_mutex_init(&shared.mutexB, &attr);
  ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_init() B failed";

  shared.flag_x = false;
  shared.flag_y = false;
  shared.flag_z = false;

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

#if 0  
  orv = osal_task_set_priority(&task_H, 3);
  ASSERT_EQ(orv, OSAL_OK) << "osal_task_set_priority() H failed";

  orv = osal_task_set_priority(&task_M, 2);
  ASSERT_EQ(orv, OSAL_OK) << "osal_task_set_priority() M failed";
#endif

  orv = osal_task_set_priority(nullptr, 1);
  if (orv != 0) {
    printf("Warning: osal_task_set_priority() L failed "
           "- consider running under \"chrt -f 1 ...\"\n");
  }

#if 0  
  orv = osal_task_set_policy(&task_H, OSAL_SCHED_POLICY_FIFO);
  ASSERT_EQ(orv, OSAL_OK) << "osal_task_set_policy() H failed";

  orv = osal_task_set_policy(&task_M, OSAL_SCHED_POLICY_FIFO);
  ASSERT_EQ(orv, OSAL_OK) << "osal_task_set_policy() M failed";
#endif

  orv = osal_task_set_policy(nullptr, OSAL_SCHED_POLICY_FIFO);
  ASSERT_EQ(orv, OSAL_OK) << "osal_task_set_policy() L failed";

  run_L(&shared);

  osal_task_retval_t trv = 0;

  orv = osal_task_join(&task_M, &trv);
  ASSERT_EQ(orv, OSAL_OK) << "osal_task_join M failed";

  orv = osal_task_join(&task_H, &trv);
  ASSERT_EQ(orv, OSAL_OK) << "osal_task_join H failed";

  orv = osal_mutex_destroy(&shared.mutexA);
  ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_destroy A failed";

  orv = osal_mutex_destroy(&shared.mutexB);
  ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_destroy B failed";

  // here, if task L inherits priority from H, time_delta
  // should be 1 sec, otherwise 10 sec.
  printf("priority inheritance test: time delta = %f\n", shared.time_delta);
  EXPECT_LT(shared.time_delta, 5.0) << (" priority adjustment failed");
}

} // namespace test_priority_inversion
