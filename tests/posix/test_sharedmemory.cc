#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "gtest/gtest.h"
#include <pthread.h>
#include <vector>

#include "libosal/osal.h"
#include "libosal/shm.h"
#include "test_utils.h"
#include <sys/mman.h>
#include <sys/resource.h>

namespace test_sharedmemory {

const int NUM_PROCESSES = 10;
const int NUM_VALUES = 10000;

const char *SEMAPHORE_NAME = "/sem_shm_checksum_test";
const char *SHM_NAME = "/shm_test1";

int verbose = 0;

typedef struct {
  uint32_t values[NUM_VALUES];
} shared_t;

size_t combine_hashes(size_t const oldhash, uint32_t const payload) {
  size_t new_hash = std::hash<uint32_t>{}(payload);
  return (oldhash << 4) ^ new_hash;
}

osal_retval_t checksum_memory() {

  osal_shm_t shm;
  shared_t *shared;
  osal_retval_t orv = {};

  osal_shm_attr_t attr = (OSAL_SHM_ATTR__FLAG__RDWR);

  orv = osal_shm_open(&shm, SHM_NAME, &attr, sizeof(shared_t));
  if (orv) {
    perror("could not open shared memory in child");
  }
  if (orv != 0) {
    return orv;
  }
  osal_shm_map_attr_t map_attr =
      (OSAL_SHM_MAP_ATTR__PROT_READ | OSAL_SHM_MAP_ATTR__PROT_WRITE |
       OSAL_SHM_MAP_ATTR__SHARED);
  orv = osal_shm_map(&shm, &map_attr, (osal_void_t **)&shared);
  if (orv != 0) {
    perror("error in child when mapping memorry");
  }
  if (orv != 0) {
    return orv;
  }

  // compute hash of memory block which was written by parent process
  size_t hash = 0;
  for (int k = 0; k < NUM_VALUES; k++) {
    hash = combine_hashes(hash, shared->values[k]);
  }

  orv = osal_shm_close(&shm);
  EXPECT_EQ(orv, 0) << "could not close shared memory";

  // write that hash to file number identified with own pid
  pid_t mypid = getpid();

  char filename[30] = {};
  snprintf(filename, sizeof(filename), "hash-sum-pid%06u.asc", mypid);
  if (verbose) {
    printf("child: for pid %i, trying to read file %s\n", mypid, filename);
  }
  FILE *f = fopen(filename, "w");
  if (f == nullptr) {
    perror("Could not open result file for writing");
  }
  EXPECT_NE(f, nullptr) << "could not open file for writing result";
  if (f == nullptr) {
    return errno;
  }
  if (f != nullptr) {
    int rv = fprintf(f, "0x%zx\n", hash);
    EXPECT_GT(rv, 0) << "write of result hash failed";
    rv = fclose(f);
    EXPECT_EQ(rv, 0) << "closing result file failed";
  }

  return orv;
}

TEST(Sharedmemory, RandomWrites) {

  int rv;
  osal_retval_t orv;
  pid_t child_pids[NUM_PROCESSES];
  shared_t *shared;

  sem_t *start_checksum_sem;

  osal_shm_t shm;

  start_checksum_sem = sem_open(SEMAPHORE_NAME, O_CREAT, S_IRWXU, 0);
  ASSERT_NE(start_checksum_sem, SEM_FAILED)
      << "test-fatal: creating pthreads semaphore failed with errno = "
      << errno;

  for (int i = 0; i < NUM_PROCESSES; i++) {
    pid_t pid = fork();
    if (pid == 0) {
      struct timespec max_time = {time(nullptr) + 10, 0};
      int rv = sem_timedwait(start_checksum_sem, &max_time);
      if (rv != 0) {
        perror("Wait failed");
        exit(1);
      }
      rv = sem_close(start_checksum_sem);

      if (rv != 0) {
        exit(errno & 0377);
      }

      osal_retval_t orv = checksum_memory();
      exit(orv & 0377); // the mask is defined by POSIX
    } else {
      child_pids[i] = pid;
    }
  }
  sleep(1);

  osal_shm_attr_t attr =
      (OSAL_SHM_ATTR__FLAG__RDWR | OSAL_SHM_ATTR__FLAG__CREAT |
       (S_IRWXU << OSAL_SHM_ATTR__MODE__SHIFT));

  orv = osal_shm_open(&shm, SHM_NAME, &attr, sizeof(shared_t));
  if (orv) {
    perror("could not open parent shared memory");
  }
  ASSERT_EQ(orv, 0) << "could not open shared memory in parent";

  osal_shm_map_attr_t map_attr =
      (OSAL_SHM_MAP_ATTR__PROT_READ | OSAL_SHM_MAP_ATTR__PROT_WRITE |
       OSAL_SHM_MAP_ATTR__SHARED);
  orv = osal_shm_map(&shm, &map_attr, (osal_void_t **)&shared);
  if (orv != 0) {
    perror("could not map shared memory in parent");
  }
  ASSERT_EQ(orv, 0) << "could not map shared memory";

  memset(shared, 0, sizeof(shared_t));

  /* write random values to shared memory,
     and compute checksum hash of them at the same time. */

  size_t hash = 0;
  srand(1);
  for (int k = 0; k < NUM_VALUES; k++) {
    uint32_t new_val = rand() % (1 << 15);
    shared->values[k] = new_val;
    hash = combine_hashes(hash, new_val);
  }
  // tell child processes to start
  for (int i = 0; i < NUM_PROCESSES; i++) {
    int rv = sem_post(start_checksum_sem);
    EXPECT_EQ(rv, 0) << "could not signal start for process" << i;
  }

  // wait for all child processes to terminate
  for (int i = 0; i < NUM_PROCESSES; i++) {
    waitpid(child_pids[i], nullptr, 0);
  }
  // close shared memory region
  orv = osal_shm_close(&shm);
  EXPECT_EQ(orv, 0) << "could not close shared memory";

  // compare stored hashes

  // since we are testing shared memory and
  // we use it in different processes,
  // the hash sum is passed in a text file.

  bool match_all = true;
  for (int i = 0; i < NUM_PROCESSES; i++) {
    size_t childhash = 0;
    char filename[30] = {};
    snprintf(filename, sizeof(filename), "hash-sum-pid%06u.asc", child_pids[i]);
    if (verbose) {
      printf("parent: for thread %i, pid %i, trying to read file %s\n", i,
             child_pids[i], filename);
    }
    FILE *f = fopen(filename, "r");
    if (f == nullptr) {
      perror("opening result file for reading failed");
    }
    EXPECT_NE((void *)f, nullptr) << "error: parent process could not open"
                                     " result file";
    if (f == nullptr) {
      break;
    }

    rv = fscanf(f, "0x%zx\n", &childhash);
    EXPECT_EQ(rv, 1) << "error: parent proc could not read result";
    fclose(f);
    unlink(filename);
    EXPECT_EQ(hash, childhash) << "memory content mismatch for file" << i;
    if (hash != childhash) {
      match_all = false;
    }
    if (verbose) {
      if (match_all) {
        printf("all shared memory comparisons matched\n");
      } else {
        printf("some memory areas did not match");
      }
    }
  }

  // tidy up semaphore
  rv = sem_close(start_checksum_sem);
  EXPECT_EQ(rv, 0) << "closing semaphore failed with errno = " << errno;

  rv = sem_unlink("/sem_shm_checksum_test");
  EXPECT_EQ(rv, 0) << "unlinking semaphore failed with errno = " << errno;
}

} // namespace test_sharedmemory

namespace test_sharedmemory_restrict {

const int NUM_PROCESSES = 10;
const int NUM_VALUES = 10000;

const char *SEMAPHORE_NAME = "/sem_shm_checksum_test2";
const char *SHM_NAME2 = "/shm_test2";
const char *PATH_SHM_NAME2 = "/dev/shm/shm_test2";

int verbose = 0;

typedef struct {
  uint32_t values[NUM_VALUES];
} shared_t;

size_t combine_hashes(size_t const oldhash, uint32_t const payload) {
  size_t new_hash = std::hash<uint32_t>{}(payload);
  return (oldhash << 4) ^ new_hash;
}

osal_retval_t checksum_memory_restrict() {

  osal_shm_t shm;
  shared_t *shared;
  osal_retval_t orv = {};

  osal_shm_attr_t attr = (OSAL_SHM_ATTR__FLAG__RDONLY);

  orv = osal_shm_open(&shm, SHM_NAME2, &attr, sizeof(shared_t));
  if (orv) {
    perror("could not open shared memory in child");
  }
  if (orv != 0) {
    return orv;
  }
  osal_shm_map_attr_t map_attr =
      (OSAL_SHM_MAP_ATTR__PROT_READ | OSAL_SHM_MAP_ATTR__SHARED);
  orv = osal_shm_map(&shm, &map_attr, (osal_void_t **)&shared);
  if (orv != 0) {
    perror("error in child when mapping memorry");
  }
  if (orv != 0) {
    return orv;
  }

  // compute hash of memory block which was written by parent process
  size_t hash = 0;
  for (int k = 0; k < NUM_VALUES; k++) {
    hash = combine_hashes(hash, shared->values[k]);
  }

  orv = osal_shm_close(&shm);
  EXPECT_EQ(orv, 0) << "could not close shared memory";

  // write that hash to file number identified with own pid
  pid_t mypid = getpid();

  char filename[30] = {};
  snprintf(filename, sizeof(filename), "hash-sum-pid%06u.asc", mypid);
  if (verbose) {
    printf("child: for pid %i, trying to read file %s\n", mypid, filename);
  }
  FILE *f = fopen(filename, "w");
  if (f == nullptr) {
    perror("Could not open result file for writing");
  }
  EXPECT_NE(f, nullptr) << "could not open file for writing result";
  if (f == nullptr) {
    return errno;
  }
  if (f != nullptr) {
    int rv = fprintf(f, "0x%zx\n", hash);
    EXPECT_GT(rv, 0) << "write of result hash failed";
    rv = fclose(f);
    EXPECT_EQ(rv, 0) << "closing result file failed";
  }

  return orv;
}

TEST(Sharedmemory, RandomWriteRestrict) {

  int rv;
  osal_retval_t orv;
  pid_t child_pids[NUM_PROCESSES];
  shared_t *shared;

  sem_t *start_checksum_sem;

  osal_shm_t shm;

  // remove any old shm file
  unlink(PATH_SHM_NAME2);
  osal_shm_attr_t attr =
      (OSAL_SHM_ATTR__FLAG__RDWR | OSAL_SHM_ATTR__FLAG__CREAT |
       OSAL_SHM_ATTR__FLAG__EXCL | OSAL_SHM_ATTR__FLAG__TRUNC |
       (S_IRWXU << OSAL_SHM_ATTR__MODE__SHIFT));

  orv = osal_shm_open(&shm, SHM_NAME2, &attr, sizeof(shared_t));
  if (orv) {
    perror("could not open parent shared memory");
  }
  ASSERT_EQ(orv, 0) << "could not open shared memory in parent";

  start_checksum_sem = sem_open(SEMAPHORE_NAME, O_CREAT, S_IRWXU, 0);
  ASSERT_NE(start_checksum_sem, SEM_FAILED)
      << "test-fatal: creating pthreads semaphore failed with errno = "
      << errno;

  for (int i = 0; i < NUM_PROCESSES; i++) {
    pid_t pid = fork();
    if (pid == 0) {
      struct timespec max_time = {time(nullptr) + 10, 0};
      int rv = sem_timedwait(start_checksum_sem, &max_time);
      if (rv != 0) {
        perror("Wait failed");
        exit(1);
      }
      rv = sem_close(start_checksum_sem);

      if (rv != 0) {
        exit(errno & 0377);
      }

      osal_retval_t orv = checksum_memory_restrict();
      exit(orv & 0377); // the mask is defined by POSIX
    } else {
      child_pids[i] = pid;
    }
  }
  sleep(1);

  osal_shm_map_attr_t map_attr =
      (OSAL_SHM_MAP_ATTR__PROT_READ | OSAL_SHM_MAP_ATTR__PROT_WRITE |
       OSAL_SHM_MAP_ATTR__SHARED);
  orv = osal_shm_map(&shm, &map_attr, (osal_void_t **)&shared);
  if (orv != 0) {
    perror("could not map shared memory in parent");
  }
  ASSERT_EQ(orv, 0) << "could not map shared memory";

  memset(shared, 0, sizeof(shared_t));

  /* write random values to shared memory,
     and compute checksum hash of them at the same time. */

  size_t hash = 0;
  srand(1);
  for (int k = 0; k < NUM_VALUES; k++) {
    uint32_t new_val = rand() % (1 << 15);
    shared->values[k] = new_val;
    hash = combine_hashes(hash, new_val);
  }
  // tell child processes to start
  for (int i = 0; i < NUM_PROCESSES; i++) {
    int rv = sem_post(start_checksum_sem);
    EXPECT_EQ(rv, 0) << "could not signal start for process" << i;
  }

  // wait for all child processes to terminate
  for (int i = 0; i < NUM_PROCESSES; i++) {
    waitpid(child_pids[i], nullptr, 0);
  }
  // close shared memory region
  orv = osal_shm_close(&shm);
  EXPECT_EQ(orv, 0) << "could not close shared memory";

  // compare stored hashes

  // since we are testing shared memory and
  // we use it in different processes,
  // the hash sum is passed in a text file.

  bool match_all = true;
  for (int i = 0; i < NUM_PROCESSES; i++) {
    size_t childhash = 0;
    char filename[30] = {};
    snprintf(filename, sizeof(filename), "hash-sum-pid%06u.asc", child_pids[i]);
    if (verbose) {
      printf("parent: for thread %i, pid %i, trying to read file %s\n", i,
             child_pids[i], filename);
    }
    FILE *f = fopen(filename, "r");
    if (f == nullptr) {
      perror("opening result file for reading failed");
    }
    EXPECT_NE((void *)f, nullptr) << "error: parent process could not open"
                                     " result file";
    if (f == nullptr) {
      break;
    }

    rv = fscanf(f, "0x%zx\n", &childhash);
    EXPECT_EQ(rv, 1) << "error: parent proc could not read result";
    fclose(f);
    unlink(filename);
    EXPECT_EQ(hash, childhash) << "memory content mismatch for file" << i;
    if (hash != childhash) {
      match_all = false;
    }
    if (verbose) {
      if (match_all) {
        printf("all shared memory comparisons matched\n");
      } else {
        printf("some memory areas did not match");
      }
    }
  }

  // tidy up semaphore
  rv = sem_close(start_checksum_sem);
  EXPECT_EQ(rv, 0) << "closing semaphore failed with errno = " << errno;

  rv = sem_unlink(SEMAPHORE_NAME);
  EXPECT_EQ(rv, 0) << "unlinking semaphore failed with errno = " << errno;
}

} // namespace test_sharedmemory_restrict

namespace test_sharedmemory_errors {

const char *SHM_NAME3 = "/shm_test3";
const char *PATH_SHM_NAME3 = "/dev/shm/shm_test3";

TEST(Sharedmemory, TestTruncateError) {

  osal_retval_t orv;
  osal_shm_t shm;

  // provoke a OSAL_ERR_PERMISION_DENIED error
  // remove any old shm file
  int rv;
  unlink(PATH_SHM_NAME3);
  // create new file
  FILE *fp = fopen(PATH_SHM_NAME3, "w");
  ASSERT_NE(fp, nullptr);
  char data[] = "abc";
  rv = fwrite(data, sizeof(data), 1, fp);
  ASSERT_GT(rv, 0);
  rv = fclose(fp);
  ASSERT_EQ(rv, 0);
  // make it read-only
  rv = chmod(PATH_SHM_NAME3, S_IRUSR);
  ASSERT_EQ(rv, 0);

  osal_shm_attr_t attr =
      (OSAL_SHM_ATTR__FLAG__RDWR | OSAL_SHM_ATTR__FLAG__TRUNC |
       (S_IRUSR << OSAL_SHM_ATTR__MODE__SHIFT));

  orv = osal_shm_open(&shm, SHM_NAME3, &attr, sizeof(long));
  if (orv) {
    perror("could not open parent shared memory");
  }
  ASSERT_EQ(orv, OSAL_ERR_PERMISSION_DENIED)
      << "could not open shared memory in parent";

  unlink(PATH_SHM_NAME3);
}

TEST(Sharedmemory, TestExistError) {

  osal_retval_t orv;
  osal_shm_t shm1;
  osal_shm_t shm2;

  // provoke a OSAL_ERR_PERMISION_DENIED error
  // remove any old shm file
  unlink(PATH_SHM_NAME3);

  osal_shm_attr_t attr1 =
      (OSAL_SHM_ATTR__FLAG__RDWR | OSAL_SHM_ATTR__FLAG__CREAT |
       (S_IRUSR << OSAL_SHM_ATTR__MODE__SHIFT));

  orv = osal_shm_open(&shm1, SHM_NAME3, &attr1, sizeof(long));
  if (orv) {
    perror("could not open parent shared memory");
  }
  ASSERT_EQ(orv, 0) << "could not open shared memory in parent";

  osal_shm_close(&shm1);

  osal_shm_attr_t attr2 =
      (OSAL_SHM_ATTR__FLAG__RDWR | OSAL_SHM_ATTR__FLAG__CREAT |
       OSAL_SHM_ATTR__FLAG__EXCL | (S_IRUSR << OSAL_SHM_ATTR__MODE__SHIFT));

  orv = osal_shm_open(&shm2, SHM_NAME3, &attr2, sizeof(long));
  if (orv) {
    perror("could not open parent shared memory");
  }
  ASSERT_EQ(orv, OSAL_ERR_OPERATION_FAILED)
      << "could not open shared memory in parent";

  unlink(PATH_SHM_NAME3);
}

TEST(Sharedmemory, TestInvalidName) {

  osal_retval_t orv;
  osal_shm_t shm;

  const char *SHM_NAME4 = "";

  // provoke a OSAL_ERR_PERMISION_DENIED error
  // remove any old shm file

  osal_shm_attr_t attr =
      (OSAL_SHM_ATTR__FLAG__RDWR | OSAL_SHM_ATTR__FLAG__CREAT |
       (S_IRUSR << OSAL_SHM_ATTR__MODE__SHIFT));

  orv = osal_shm_open(&shm, SHM_NAME4, &attr, sizeof(long));
  if (orv) {
    perror("could not open parent shared memory");
  }
  ASSERT_EQ(orv, OSAL_ERR_INVALID_PARAM)
      << "could not open shared memory in parent";
}

TEST(Sharedmemory, TestTooLongName) {

  osal_retval_t orv;
  osal_shm_t shm;
  // use a name that is likely longer than PATH_MAX
  const int NAME_LENGTH = 50000;

  char shm_name6[NAME_LENGTH];
  char initc = 'a';

  memset(shm_name6, initc, NAME_LENGTH);
  shm_name6[NAME_LENGTH - 1] = '\0';

  // provoke a OSAL_ERR_PERMISION_DENIED error
  // remove any old shm file

  osal_shm_attr_t attr =
      (OSAL_SHM_ATTR__FLAG__RDWR | OSAL_SHM_ATTR__FLAG__CREAT |
       (S_IRUSR << OSAL_SHM_ATTR__MODE__SHIFT));

  orv = osal_shm_open(&shm, shm_name6, &attr, sizeof(long));
  if (orv) {
    perror("could not open parent shared memory");
  }
  ASSERT_EQ(orv, OSAL_ERR_INVALID_PARAM)
      << "could not open shared memory in parent";
}

TEST(Sharedmemory, TestTooManyFiles) {

  osal_retval_t orv;
  int rv;
  osal_shm_t shm;
  struct rlimit lim;
  struct rlimit old_lim;

  const char *SHM_NAME5 = "shm_test5";
  const char *PATH_SHM_NAME5 = "/dev/shm/shm_test5";

  // provoke a OSAL_ERR_PERMISION_DENIED error
  // remove any old shm file
  unlink(PATH_SHM_NAME5);

  osal_shm_attr_t attr =
      (OSAL_SHM_ATTR__FLAG__RDWR | OSAL_SHM_ATTR__FLAG__CREAT |
       (S_IRUSR << OSAL_SHM_ATTR__MODE__SHIFT));

  rv = getrlimit(RLIMIT_NOFILE, &lim);
  ASSERT_EQ(rv, 0) << "could not get rlimit";
  old_lim = lim;
  lim.rlim_cur = 0;
  rv = setrlimit(RLIMIT_NOFILE, &lim);
  ASSERT_EQ(rv, 0) << "could not set rlimit";

  orv = osal_shm_open(&shm, SHM_NAME5, &attr, sizeof(long));
  if (orv) {
    perror("could not open parent shared memory");
  }
  rv = setrlimit(RLIMIT_NOFILE, &old_lim);
  ASSERT_EQ(rv, 0) << "could not restore rlimit";

  ASSERT_EQ(orv, OSAL_ERR_SYSTEM_LIMIT_REACHED)
      << "could not open shared memory in parent";
}

} // namespace test_sharedmemory_errors

namespace test_mmap {

TEST(Sharedmemory, TestProtExec) {

  const char *SHM_NAME7 = "shm_test7";
  const char *PATH_SHM_NAME7 = "/dev/shm/shmd_test7";

  osal_retval_t orv;
  osal_shm_t shm;
  long *p_mem;

  osal_shm_attr_t attr =
      (OSAL_SHM_ATTR__FLAG__RDWR | OSAL_SHM_ATTR__FLAG__CREAT |
       ((S_IRUSR | S_IWUSR) << OSAL_SHM_ATTR__MODE__SHIFT));

  orv = osal_shm_open(&shm, SHM_NAME7, &attr, sizeof(long));
  if (orv) {
    perror("could not open parent shared memory");
  }
  ASSERT_EQ(orv, 0) << "could not open shared memory in parent";

  osal_shm_map_attr_t map_attr =
      (OSAL_SHM_MAP_ATTR__PROT_READ | OSAL_SHM_MAP_ATTR__PROT_EXEC |
       OSAL_SHM_MAP_ATTR__SHARED);
  orv = osal_shm_map(&shm, &map_attr, (osal_void_t **)&p_mem);
  if (orv != 0) {
    perror("could not map shared memory in parent");
  }
  ASSERT_EQ(orv, 0) << "could not map shared memory";

  orv = osal_shm_close(&shm);
  EXPECT_EQ(orv, 0) << "could not close shared memory";

  unlink(PATH_SHM_NAME7);
}

TEST(Sharedmemory, TestProtNone) {

  const char *SHM_NAME7 = "shm_test7";
  const char *PATH_SHM_NAME7 = "/dev/shm/shmd_test7";

  osal_retval_t orv;
  osal_shm_t shm;
  long *p_mem;

  osal_shm_attr_t attr =
      (OSAL_SHM_ATTR__FLAG__RDWR | OSAL_SHM_ATTR__FLAG__CREAT |
       ((S_IRUSR | S_IWUSR) << OSAL_SHM_ATTR__MODE__SHIFT));

  orv = osal_shm_open(&shm, SHM_NAME7, &attr, sizeof(long));
  if (orv) {
    perror("could not open parent shared memory");
  }
  ASSERT_EQ(orv, 0) << "could not open shared memory in parent";

  osal_shm_map_attr_t map_attr =
      (OSAL_SHM_MAP_ATTR__PROT_READ | OSAL_SHM_MAP_ATTR__PROT_NONE |
       OSAL_SHM_MAP_ATTR__SHARED);
  orv = osal_shm_map(&shm, &map_attr, (osal_void_t **)&p_mem);
  if (orv != 0) {
    perror("could not map shared memory in parent");
  }
  ASSERT_EQ(orv, 0) << "could not map shared memory";

  orv = osal_shm_close(&shm);
  EXPECT_EQ(orv, 0) << "could not close shared memory";

  unlink(PATH_SHM_NAME7);
}

TEST(Sharedmemory, TestProtPrivate) {

  const char *SHM_NAME7 = "shm_test7";
  const char *PATH_SHM_NAME7 = "/dev/shm/shmd_test7";

  osal_retval_t orv;
  osal_shm_t shm;
  long *p_mem;

  osal_shm_attr_t attr =
      (OSAL_SHM_ATTR__FLAG__RDWR | OSAL_SHM_ATTR__FLAG__CREAT |
       ((S_IRUSR | S_IWUSR) << OSAL_SHM_ATTR__MODE__SHIFT));

  orv = osal_shm_open(&shm, SHM_NAME7, &attr, sizeof(long));
  if (orv) {
    perror("could not open parent shared memory");
  }
  ASSERT_EQ(orv, 0) << "could not open shared memory in parent";

  osal_shm_map_attr_t map_attr =
      (OSAL_SHM_MAP_ATTR__PROT_READ | OSAL_SHM_MAP_ATTR__PRIVATE |
       OSAL_SHM_MAP_ATTR__SHARED);
  orv = osal_shm_map(&shm, &map_attr, (osal_void_t **)&p_mem);
  if (orv != 0) {
    perror("could not map shared memory in parent");
  }
  ASSERT_EQ(orv, 0) << "could not map shared memory";

  orv = osal_shm_close(&shm);
  EXPECT_EQ(orv, 0) << "could not close shared memory";

  unlink(PATH_SHM_NAME7);
}

TEST(Sharedmemory, TestMMAPWorks) {

  const char *SHM_NAME7 = "shm_test7";
  const char *PATH_SHM_NAME7 = "/dev/shm/shmd_test7";

  osal_retval_t orv;
  osal_shm_t shm;
  long *p_mem;

  osal_shm_attr_t attr =
      (OSAL_SHM_ATTR__FLAG__RDWR | OSAL_SHM_ATTR__FLAG__CREAT |
       ((S_IRUSR | S_IWUSR) << OSAL_SHM_ATTR__MODE__SHIFT));

  orv = osal_shm_open(&shm, SHM_NAME7, &attr, sizeof(long));
  if (orv) {
    perror("could not open parent shared memory");
  }
  ASSERT_EQ(orv, 0) << "could not open shared memory in parent";

  osal_shm_map_attr_t map_attr =
      (OSAL_SHM_MAP_ATTR__PROT_WRITE | OSAL_SHM_MAP_ATTR__SHARED);
  orv = osal_shm_map(&shm, &map_attr, (osal_void_t **)&p_mem);
  if (orv != 0) {
    perror("could not map shared memory in parent");
  }
  ASSERT_EQ(orv, 0) << "could not map shared memory";

  orv = osal_shm_close(&shm);
  EXPECT_EQ(orv, 0) << "could not close shared memory";

  unlink(PATH_SHM_NAME7);
}

TEST(Sharedmemory, TestPermDenied) {

  const char *SHM_NAME7 = "shm_test7";
  const char *PATH_SHM_NAME7 = "/dev/shm/shmd_test7";

  osal_retval_t orv;
  osal_shm_t shm;
  long *p_mem;

  osal_shm_attr_t attr =
      (OSAL_SHM_ATTR__FLAG__RDONLY | OSAL_SHM_ATTR__FLAG__CREAT |
       ((S_IRUSR | S_IWUSR) << OSAL_SHM_ATTR__MODE__SHIFT));

  orv = osal_shm_open(&shm, SHM_NAME7, &attr, sizeof(long));
  if (orv) {
    perror("could not open parent shared memory");
  }
  ASSERT_EQ(orv, 0) << "could not open shared memory in parent";

  osal_shm_map_attr_t map_attr =
      (OSAL_SHM_MAP_ATTR__PROT_WRITE | OSAL_SHM_MAP_ATTR__SHARED);
  orv = osal_shm_map(&shm, &map_attr, (osal_void_t **)&p_mem);
  if (orv != 0) {
    perror("could not map shared memory in parent");
  }
  ASSERT_EQ(orv, OSAL_ERR_PERMISSION_DENIED) << "could not map shared memory";

  orv = osal_shm_close(&shm);
  EXPECT_EQ(orv, 0) << "could not close shared memory";

  unlink(PATH_SHM_NAME7);
}

TEST(Sharedmemory, TestMmapSizeLimit) {

  const char *SHM_NAME7 = "shm_test7";
  const char *PATH_SHM_NAME7 = "/dev/shm/shmd_test7";

  osal_retval_t orv;
  int rv;
  osal_shm_t shm;
  struct rlimit lim;
  struct rlimit old_lim;
  long arr[100000];
  int lock_result;

  osal_shm_attr_t attr =
      (OSAL_SHM_ATTR__FLAG__RDWR | OSAL_SHM_ATTR__FLAG__CREAT |
       ((S_IRUSR | S_IWUSR) << OSAL_SHM_ATTR__MODE__SHIFT));

  orv = osal_shm_open(&shm, SHM_NAME7, &attr, sizeof(arr));
  if (orv) {
    perror("could not open parent shared memory");
  }
  ASSERT_EQ(orv, 0) << "could not open shared memory in parent";

  rv = getrlimit(RLIMIT_MEMLOCK, &lim);
  ASSERT_EQ(rv, 0) << "could not get rlimit";
  old_lim = lim;
  lim.rlim_cur = 0;
  rv = setrlimit(RLIMIT_MEMLOCK, &lim);
  ASSERT_EQ(rv, 0) << "could not set rlimit";

  lock_result = mlockall(MCL_FUTURE);
  if (lock_result) {
    printf("info: could not lock memory\n");
  }

  osal_shm_map_attr_t map_attr =
      (OSAL_SHM_MAP_ATTR__PROT_WRITE | OSAL_SHM_MAP_ATTR__SHARED);
  orv = osal_shm_map(&shm, &map_attr, (osal_void_t **)&arr);
  if (orv != 0) {
    perror("could not map shared memory in parent");
  }
  if (!lock_result) {
    ASSERT_EQ(orv, 0) << "could not map shared memory";
  } else {
    ASSERT_EQ(orv, 0) << "could not map shared memory";
  }

  rv = setrlimit(RLIMIT_MEMLOCK, &old_lim);
  ASSERT_EQ(rv, 0) << "could not set rlimit";

  orv = osal_shm_close(&shm);
  EXPECT_EQ(orv, 0) << "could not close shared memory";

  unlink(PATH_SHM_NAME7);
}

TEST(Sharedmemory, TestBadDescriptor) {

  const char *SHM_NAME7 = "shm_test7";
  const char *PATH_SHM_NAME7 = "/dev/shm/shmd_test7";

  osal_retval_t orv;
  osal_shm_t shm1;
  osal_shm_t shm2;
  long *p_data;

  osal_shm_attr_t attr =
      (OSAL_SHM_ATTR__FLAG__RDWR | OSAL_SHM_ATTR__FLAG__CREAT |
       ((S_IRUSR | S_IWUSR) << OSAL_SHM_ATTR__MODE__SHIFT));

  orv = osal_shm_open(&shm1, SHM_NAME7, &attr, sizeof(long));
  if (orv) {
    perror("could not open parent shared memory");
  }
  ASSERT_EQ(orv, 0) << "could not open shared memory in parent";

  shm2 = shm1;
  shm2.fd = -1;

  osal_shm_map_attr_t map_attr =
      (OSAL_SHM_MAP_ATTR__PROT_WRITE | OSAL_SHM_MAP_ATTR__SHARED);
  orv = osal_shm_map(&shm2, &map_attr, (osal_void_t **)&p_data);
  if (orv != 0) {
    perror("could not map shared memory in parent");
  }
  ASSERT_EQ(orv, OSAL_ERR_INVALID_PARAM) << "could not map shared memory";

  orv = osal_shm_close(&shm1);
  EXPECT_EQ(orv, 0) << "could not close shared memory";

  unlink(PATH_SHM_NAME7);
}

} // end namespace test_mmap

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  if (getenv("VERBOSE")) {
    test_sharedmemory::verbose = 1;
  }

  return RUN_ALL_TESTS();
}
