
#include "gtest/gtest.h"
#include <pthread.h>
#include <vector>

#include "libosal/mutex.h"
#include "libosal/osal.h"
#include "test_utils.h"

namespace test_mutex {

using testutils::wait_nanoseconds;

int verbose = 0;

TEST(MutexFunction, SingleThreadedNoRelease) {
  osal_mutex_t my_mutex;
  osal_mutex_init(&my_mutex, nullptr);
  osal_mutex_lock(&my_mutex);
  const int loopcount = 100;
  int counter = 0;

  for (int i = 0; i < loopcount; i++) {
    counter += 1;
  }

  osal_mutex_unlock(&my_mutex);
  osal_mutex_destroy(&my_mutex);

  EXPECT_EQ(counter, loopcount) << (" sanity test failed, something "
                                    "is totally wrong");
}

TEST(MutexFunction, SingleThreadedWithRelease) {
  osal_mutex_t my_mutex;
  osal_mutex_init(&my_mutex, nullptr);
  const int loopcount = 100;
  int counter = 0;

  for (int i = 0; i < loopcount; i++) {
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

void *test_random(void *p_params) {
  thread_param_t params = *((thread_param_t *)p_params);

  const int thread_id = params.thread_id;
  const uint max_wait_time = params.max_wait_time_nsec;
  osal_retval_t orv;

  if (max_wait_time > 0) {
    srand(thread_id);
  }
  for (uint i = 0; i < params.loopcount; i++) {
    // randomly wait
    if ((max_wait_time > 0) && (rand() % 2)) {
      wait_nanoseconds(rand() % max_wait_time);
    }

    orv = osal_mutex_lock(params.p_count_mutex);
    EXPECT_EQ(orv, OSAL_OK) << "osal_mutex_lock() failed";

    unsigned long old_value = *(params.p_counter);
    // randomly wait
    if ((max_wait_time > 0) && (rand() % 2)) {
      wait_nanoseconds(rand() % max_wait_time);
    }

    // increment shared counter
    *(params.p_counter) = old_value + 1;

    // return lock
    orv = osal_mutex_unlock(params.p_count_mutex);
    EXPECT_EQ(orv, OSAL_OK) << "osal_mutex_unlock() failed";
  }

  return nullptr;
}

TEST(MutexFunction, ParallelMultiThreading) {
  const ulong N_THREADS = 100;
  const uint LOOPCOUNT = 100000;

  pthread_t thread_ids[N_THREADS];
  thread_param_t thread_params[N_THREADS];
  osal_mutex_t count_mutex;
  unsigned long counter = 0;
  osal_retval_t orv;
  int rv;

  bool verbose = (getenv("VERBOSE") != nullptr);

  orv = osal_mutex_init(&count_mutex, nullptr);
  ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_init() failed";

  for (ulong i = 0; i < N_THREADS; i++) {
    thread_params[i].thread_id = i;
    thread_params[i].p_count_mutex = &count_mutex;
    thread_params[i].p_counter = &counter;
    thread_params[i].loopcount = LOOPCOUNT;
    thread_params[i].max_wait_time_nsec = 0;

    if (verbose) {
      printf("starting thread %lu\n", i);
    }
    rv = pthread_create(/*thread*/ &(thread_ids[i]),
                        /*pthread_attr*/ nullptr,
                        /* start_routine */ test_random,
                        /* arg */ (void *)&(thread_params[i]));
    ASSERT_EQ(rv, 0) << "pthread_create() failed";
  }
  for (ulong i = 0; i < N_THREADS; i++) {
    if (verbose) {
      printf("joining thread %lu\n", i);
    }
    rv = pthread_join(/*thread*/ thread_ids[i],
                      /*retval*/ nullptr);
    ASSERT_EQ(rv, 0) << "pthread_join() failed";
  }
  orv = osal_mutex_destroy(&count_mutex);
  ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_init() failed";

  if (verbose) {
    printf("expected counts: %lu, actual counter: %lu \n",
           N_THREADS * LOOPCOUNT, counter);
  }

  EXPECT_EQ(counter, N_THREADS * LOOPCOUNT)
      << "multi-threaded counter test failed";
}

TEST(MutexFunction, MultithreadingPlusRandomizedWait) {
  const ulong N_THREADS = 8;
  const uint LOOPCOUNT = 10000;
  const uint MAX_WAIT_TIME_NSEC = 500;

  pthread_t thread_ids[N_THREADS];
  thread_param_t thread_params[N_THREADS];
  osal_mutex_t count_mutex;
  unsigned long counter = 0;

  bool verbose = (getenv("VERBOSE") != nullptr);

  osal_mutex_init(&count_mutex, nullptr);

  for (ulong i = 0; i < N_THREADS; i++) {
    thread_params[i].thread_id = i;
    thread_params[i].p_count_mutex = &count_mutex;
    thread_params[i].p_counter = &counter;
    thread_params[i].loopcount = LOOPCOUNT;
    thread_params[i].max_wait_time_nsec = MAX_WAIT_TIME_NSEC;

    if (verbose) {
      printf("starting thread %lu\n", i);
    }
    pthread_create(/*thread*/ &(thread_ids[i]),
                   /*pthread_attr*/ nullptr,
                   /* start_routine */ test_random,
                   /* arg */ (void *)&(thread_params[i]));
  }
  // should complete in about 50 ms
  for (ulong i = 0; i < N_THREADS; i++) {
    if (verbose) {
      printf("joining thread %lu\n", i);
    }
    pthread_join(/*thread*/ thread_ids[i],
                 /*retval*/ nullptr);
  }
  osal_mutex_destroy(&count_mutex);

  if (verbose) {
    printf("expected counts: %lu, actual counter: %lu \n",
           N_THREADS * LOOPCOUNT, counter);
  }

  EXPECT_EQ(counter, N_THREADS * LOOPCOUNT)
      << "multi-threaded counter test failed";
}

TEST(MutexFunction, TryLock) {
  osal_mutex_t my_mutex;
  osal_retval_t orv;

  orv = osal_mutex_init(&my_mutex, nullptr);
  EXPECT_EQ(orv, OSAL_OK) << "osal_mutex_init() failed";

  orv = osal_mutex_lock(&my_mutex);
  EXPECT_EQ(orv, OSAL_OK) << "osal_mutex_lock() failed";

  orv = osal_mutex_trylock(&my_mutex);
  EXPECT_EQ(orv, OSAL_ERR_BUSY) << "osal_mutex_trylock() has wrong result!";

  orv = osal_mutex_unlock(&my_mutex);
  EXPECT_EQ(orv, OSAL_OK) << "osal_mutex_unlock() failed";

  orv = osal_mutex_trylock(&my_mutex);
  EXPECT_EQ(orv, OSAL_OK)
      << "osal_mutex_trylock() failed in spite of free lock";

  orv = osal_mutex_unlock(&my_mutex);
  EXPECT_EQ(orv, OSAL_OK) << "osal_mutex_unlock() failed";

  orv = osal_mutex_destroy(&my_mutex);
  ASSERT_EQ(orv, OSAL_OK) << "osal_mutex_destroy() failed";
}

TEST(MutexDetect, TestRelock) {
  osal_mutex_t my_mutex;
  osal_mutex_attr_t attr;
  osal_retval_t orv = {};

  attr = OSAL_MUTEX_ATTR__TYPE__ERRORCHECK;

  orv = osal_mutex_init(&my_mutex, &attr);
  ASSERT_EQ(orv, 0) << "Could not initialize mutex";
  orv = osal_mutex_lock(&my_mutex);
  ASSERT_EQ(orv, 0) << "Could not initialize mutex";

  // re-lock mutex, which is defined for an error-checking mutex

  orv = osal_mutex_lock(&my_mutex);
  EXPECT_EQ(orv, OSAL_ERR_DEAD_LOCK) << "Could re-lock mutex";

  orv = osal_mutex_unlock(&my_mutex);
  EXPECT_EQ(orv, 0) << "Could not unlock mutex";

  orv = osal_mutex_destroy(&my_mutex);
  EXPECT_EQ(orv, 0) << "Could not destroy mutex";
}

void *lock_thread(void *p_params) {
  osal_mutex_t *p_mutex = (osal_mutex_t *)p_params;
  osal_retval_t orv = {};

  orv = osal_mutex_lock(p_mutex);
  if (orv != 0) {
    printf("could not lock mutex - return value %i \n", (int)orv);
  } else {
    if (verbose) {
      printf("locked mutex, ok \n");
    }
  }
  return nullptr;
}

TEST(MutexDetect, OwnerDead1) {
  osal_mutex_t my_mutex;
  osal_mutex_attr_t attr;
  osal_retval_t orv = {};
  int rv = 0;
  pthread_t thread_id;

  attr = OSAL_MUTEX_ATTR__TYPE__ERRORCHECK | OSAL_MUTEX_ATTR__ROBUST;

  orv = osal_mutex_init(&my_mutex, &attr);
  ASSERT_EQ(orv, 0) << "Could not initialize mutex";

  rv = pthread_create(/*thread*/ &thread_id,
                      /*pthread_attr*/ nullptr,
                      /* start_routine */ lock_thread,
                      /* arg */ (void *)&(my_mutex));
  ASSERT_EQ(rv, 0) << "pthread_create() failed";
  rv = pthread_join(thread_id, nullptr);
  ASSERT_EQ(rv, 0) << "pthread_join() failed";

  if (verbose) {
    printf("thread joined, locking...\n");
  }
  orv = osal_mutex_lock(&my_mutex);
  EXPECT_EQ(orv, OSAL_ERR_OWNER_DEAD) << "Could lock orphaned mutex";

  orv = osal_mutex_destroy(&my_mutex);
  EXPECT_EQ(orv, 0) << "Could not destroy mutex";
}

TEST(MutexDetect, OwnerDead2) {
  osal_mutex_t my_mutex;
  osal_mutex_attr_t attr;
  osal_retval_t orv = {};
  int rv = 0;
  pthread_t thread_id;

  attr = OSAL_MUTEX_ATTR__TYPE__ERRORCHECK | OSAL_MUTEX_ATTR__ROBUST;

  orv = osal_mutex_init(&my_mutex, &attr);
  ASSERT_EQ(orv, 0) << "Could not initialize mutex";

  rv = pthread_create(/*thread*/ &thread_id,
                      /*pthread_attr*/ nullptr,
                      /* start_routine */ lock_thread,
                      /* arg */ (void *)&(my_mutex));
  ASSERT_EQ(rv, 0) << "pthread_create() failed";
  rv = pthread_join(thread_id, nullptr);
  ASSERT_EQ(rv, 0) << "pthread_join() failed";

  if (verbose) {
    printf("thread joined, locking...\n");
  }

  orv = osal_mutex_trylock(&my_mutex);
  EXPECT_EQ(orv, OSAL_ERR_OWNER_DEAD) << "Could lock orphaned mutex";

  orv = osal_mutex_destroy(&my_mutex);
  EXPECT_EQ(orv, 0) << "Could not destroy mutex";
}

TEST(MutexFunction, InheritPar) {
  osal_mutex_t my_mutex;
  osal_mutex_attr_t attr;
  osal_retval_t orv = {};

  attr = OSAL_MUTEX_ATTR__PROTOCOL__INHERIT;

  orv = osal_mutex_init(&my_mutex, &attr);
  ASSERT_EQ(orv, 0) << "Could not initialize mutex";

  orv = osal_mutex_lock(&my_mutex);
  EXPECT_EQ(orv, 0) << "Could not lock priority-inheritance mutex";

  orv = osal_mutex_unlock(&my_mutex);
  EXPECT_EQ(orv, 0) << "Could not unlock  priority-inheritance mutex";

  orv = osal_mutex_destroy(&my_mutex);
  EXPECT_EQ(orv, 0) << "Could not destroy  priority-inheritance mutex";
}

TEST(MutexFunction, ProtectPar) {
  osal_mutex_t my_mutex;
  osal_mutex_attr_t attr;
  osal_retval_t orv = {};

  attr = OSAL_MUTEX_ATTR__PROTOCOL__PROTECT |
         (1u << OSAL_MUTEX_ATTR__PRIOCEILING__SHIFT);

  orv = osal_mutex_init(&my_mutex, &attr);
  ASSERT_EQ(orv, 0) << "Could not initialize mutex";

  orv = osal_mutex_lock(&my_mutex);
  EXPECT_EQ(orv, OSAL_ERR_INVALID_PARAM) << "Could not lock prio-protect mutex";

  if (orv == 0) {
    orv = osal_mutex_unlock(&my_mutex);
    EXPECT_EQ(orv, 0) << "Could not unlock prio-protect mutex";
  }

  orv = osal_mutex_destroy(&my_mutex);
  EXPECT_EQ(orv, 0) << "Could not destroy mutex";
}

TEST(MutexFunction, TestRecursive) {
  osal_mutex_t my_mutex;
  osal_mutex_attr_t attr;
  osal_retval_t orv = {};

  attr = OSAL_MUTEX_ATTR__TYPE__RECURSIVE;

  orv = osal_mutex_init(&my_mutex, &attr);
  ASSERT_EQ(orv, 0) << "Could not initialize mutex";
  orv = osal_mutex_lock(&my_mutex);
  ASSERT_EQ(orv, 0) << "Could not initialize mutex";

  // re-lock mutex, which works for a recursive mutex

  orv = osal_mutex_lock(&my_mutex);
  EXPECT_EQ(orv, 0) << "Could re-lock mutex";

  // need to unlock two times here
  orv = osal_mutex_unlock(&my_mutex);
  EXPECT_EQ(orv, 0) << "Could not unlock mutex";

  orv = osal_mutex_unlock(&my_mutex);
  EXPECT_EQ(orv, 0) << "Could not unlock mutex";

  orv = osal_mutex_destroy(&my_mutex);
  EXPECT_EQ(orv, 0) << "Could not destroy mutex";
}

} // namespace test_mutex

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  if (getenv("VERBOSE")) {
    test_mutex::verbose = 1;
  }

  return RUN_ALL_TESTS();
}
