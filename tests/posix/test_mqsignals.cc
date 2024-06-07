#include "libosal/mq.h"
#include "libosal/osal.h"
#include "test_utils.h"
#include "gtest/gtest.h"
#include <pthread.h>
#include <signal.h>
#include <sys/resource.h>
#include <unistd.h>
#include <vector>

namespace test_mqsignals {

/* this part of the message queue tests is in a separate program
   because it sets signal handlers, which can interfere
   with other tests.
*/

int verbose = 0;

using testutils::set_deadline;
using testutils::wait_nanoseconds;

namespace test_interrupts {

void handle_test_signal(int s) { printf("received signal %i\n", s); }

TEST(MessageQueueFunction, TestInterruptSend) {

  int rv;
  osal_retval_t orv;
  osal_mq_t mqueue;
  unsigned char buf[16];

  // initialize message queue
  osal_mq_attr_t attr = {};
  attr.oflags = OSAL_MQ_ATTR__OFLAG__RDWR | OSAL_MQ_ATTR__OFLAG__CREAT;
  attr.max_messages = 1;
  ASSERT_GE(attr.max_messages, 0u);
  attr.max_message_size = 16;
  ASSERT_GE(attr.max_message_size, 0u);
  attr.mode = S_IRUSR | S_IWUSR;

  /* test with a too large buffer */

  // unlink message queue if it exists.
  // Note: the return value is intentionally not checked.
  mq_unlink("/test10");

  orv = osal_mq_open(&mqueue, "/test10", &attr);
  if (orv != 0) {
    perror("failed to open mq:");
  }
  EXPECT_EQ(orv, OSAL_OK) << "osal_mq_open() failed";

  memset(&buf, 1, sizeof(buf));

  pid_t mainpid = getpid();
  if (fork() == 0) {
    sleep(1);
    kill(mainpid, SIGUSR1);
  } else {

    struct sigaction act = {};
    struct sigaction oldact = {};
    // act.sa_flags = SA_RESTART;
    act.sa_handler = &handle_test_signal;
    rv = sigaction(SIGUSR1, &act, &oldact);
    if (rv) {
      perror("error setting sigaction");
    }
    ASSERT_EQ(rv, 0);

    osal_uint32_t prio = 1;
    orv = osal_mq_send(&mqueue, (const osal_char_t *)&buf, sizeof(buf), prio);

    EXPECT_EQ(orv, OSAL_OK) << "osal_mq_send() failed";

    orv = osal_mq_send(&mqueue, (const osal_char_t *)&buf, sizeof(buf), prio);
    // we expect this to fail because of the signal sent,
    // which interrupts the call
    EXPECT_EQ(orv, OSAL_ERR_INTERRUPTED) << "osal_mq_send() failed";

    rv = sigaction(SIGUSR1, &oldact, nullptr);
    if (rv) {
      perror("error resetting sigaction");
    }
    ASSERT_EQ(rv, 0);
  }

  orv = osal_mq_close(&mqueue);
  if (orv != 0) {
    perror("failed to close mq:");
  }
  EXPECT_EQ(orv, OSAL_OK) << "osal_mq_close() failed";

  printf("mq_receive orv = %i\n", orv);
}

TEST(MessageQueueFunction, TestInterruptTimedSend) {

  int rv;
  osal_retval_t orv;
  osal_mq_t mqueue;
  unsigned char buf[16];

  // initialize message queue
  osal_mq_attr_t attr = {};
  attr.oflags = OSAL_MQ_ATTR__OFLAG__RDWR | OSAL_MQ_ATTR__OFLAG__CREAT;
  attr.max_messages = 1;
  ASSERT_GE(attr.max_messages, 0u);
  attr.max_message_size = 16;
  ASSERT_GE(attr.max_message_size, 0u);
  attr.mode = S_IRUSR | S_IWUSR;

  /* test with a too large buffer */

  // unlink message queue if it exists.
  // Note: the return value is intentionally not checked.
  mq_unlink("/test11");

  orv = osal_mq_open(&mqueue, "/test11", &attr);
  if (orv != 0) {
    perror("failed to open mq:");
  }
  EXPECT_EQ(orv, OSAL_OK) << "osal_mq_open() failed";

  memset(&buf, 1, sizeof(buf));

  pid_t mainpid = getpid();
  if (fork() == 0) {
    sleep(1);
    kill(mainpid, SIGUSR1);
  } else {

    struct sigaction act = {};
    struct sigaction oldact = {};
    // act.sa_flags = SA_RESTART;
    act.sa_handler = &handle_test_signal;
    rv = sigaction(SIGUSR1, &act, &oldact);
    if (rv) {
      perror("error setting sigaction");
    }
    ASSERT_EQ(rv, 0);

    osal_uint32_t prio = 1;
    orv = osal_mq_send(&mqueue, (const osal_char_t *)&buf, sizeof(buf), prio);

    EXPECT_EQ(orv, OSAL_OK) << "osal_mq_send() failed";

    osal_timer_t deadline = set_deadline(2, 0);
    orv = osal_mq_timedsend(&mqueue, (const osal_char_t *)&buf, sizeof(buf),
                            prio, &deadline);
    // we expect this to fail with timeout, because of is ignored
    EXPECT_EQ(orv, OSAL_ERR_TIMEOUT) << "osal_mq_send() failed";

    rv = sigaction(SIGUSR1, &oldact, nullptr);
    if (rv) {
      perror("error resetting sigaction");
    }
    ASSERT_EQ(rv, 0);
  }

  orv = osal_mq_close(&mqueue);
  if (orv != 0) {
    perror("failed to close mq:");
  }
  EXPECT_EQ(orv, OSAL_OK) << "osal_mq_close() failed";
}

TEST(MessageQueueFunction, TestInterruptReceive) {

  int rv;
  osal_retval_t orv;
  osal_mq_t mqueue;
  unsigned char buf[16];

  // initialize message queue
  osal_mq_attr_t attr = {};
  attr.oflags = OSAL_MQ_ATTR__OFLAG__RDWR | OSAL_MQ_ATTR__OFLAG__CREAT;
  attr.max_messages = 1;
  ASSERT_GE(attr.max_messages, 0u);
  attr.max_message_size = 16;
  ASSERT_GE(attr.max_message_size, 0u);
  attr.mode = S_IRUSR | S_IWUSR;

  /* test with a too large buffer */

  // unlink message queue if it exists.
  // Note: the return value is intentionally not checked.
  mq_unlink("/test11");

  orv = osal_mq_open(&mqueue, "/test11", &attr);
  if (orv != 0) {
    perror("failed to open mq:");
  }
  EXPECT_EQ(orv, OSAL_OK) << "osal_mq_open() failed";

  memset(&buf, 1, sizeof(buf));

  pid_t mainpid = getpid();
  if (fork() == 0) {
    sleep(1);
    kill(mainpid, SIGUSR1);
  } else {

    struct sigaction act = {};
    struct sigaction oldact = {};
    // act.sa_flags = SA_RESTART;
    act.sa_handler = &handle_test_signal;
    rv = sigaction(SIGUSR1, &act, &oldact);
    if (rv) {
      perror("error setting sigaction");
    }
    ASSERT_EQ(rv, 0);

    osal_uint32_t prio = 1;
    orv = osal_mq_receive(&mqueue, (osal_char_t *)&buf, sizeof(buf), &prio);

    EXPECT_EQ(orv, OSAL_ERR_INTERRUPTED) << "osal_mq_receive() failed";

    rv = sigaction(SIGUSR1, &oldact, nullptr);
    if (rv) {
      perror("error resetting sigaction");
    }
    ASSERT_EQ(rv, 0);
  }

  orv = osal_mq_close(&mqueue);
  if (orv != 0) {
    perror("failed to close mq:");
  }
  EXPECT_EQ(orv, OSAL_OK) << "osal_mq_close() failed";
}

TEST(MessageQueueFunction, TestInterruptTimedReceive) {

  int rv;
  osal_retval_t orv;
  osal_mq_t mqueue;
  unsigned char buf[16];

  // initialize message queue
  osal_mq_attr_t attr = {};
  attr.oflags = OSAL_MQ_ATTR__OFLAG__RDWR | OSAL_MQ_ATTR__OFLAG__CREAT;
  attr.max_messages = 1;
  ASSERT_GE(attr.max_messages, 0u);
  attr.max_message_size = 16;
  ASSERT_GE(attr.max_message_size, 0u);
  attr.mode = S_IRUSR | S_IWUSR;

  /* test with a too large buffer */

  // unlink message queue if it exists.
  // Note: the return value is intentionally not checked.
  mq_unlink("/test12");

  orv = osal_mq_open(&mqueue, "/test12", &attr);
  if (orv != 0) {
    perror("failed to open mq:");
  }
  EXPECT_EQ(orv, OSAL_OK) << "osal_mq_open() failed";

  memset(&buf, 1, sizeof(buf));

  pid_t mainpid = getpid();
  if (fork() == 0) {
    sleep(1);
    kill(mainpid, SIGUSR1);
  } else {

    struct sigaction act = {};
    struct sigaction oldact = {};
    // act.sa_flags = SA_RESTART;
    act.sa_handler = &handle_test_signal;
    rv = sigaction(SIGUSR1, &act, &oldact);
    if (rv) {
      perror("error setting sigaction");
    }
    ASSERT_EQ(rv, 0);

    osal_uint32_t prio = 1;

    osal_timer_t deadline = set_deadline(2, 0);
    orv = osal_mq_timedreceive(&mqueue, (osal_char_t *)&buf, sizeof(buf), &prio,
                               &deadline);
    // we expect this to fail with timeout, because of is ignored
    EXPECT_EQ(orv, OSAL_ERR_TIMEOUT) << "osal_mq_send() failed";

    rv = sigaction(SIGUSR1, &oldact, nullptr);
    if (rv) {
      perror("error resetting sigaction");
    }
    ASSERT_EQ(rv, 0);
  }

  orv = osal_mq_close(&mqueue);
  if (orv != 0) {
    perror("failed to close mq:");
  }
  EXPECT_EQ(orv, OSAL_OK) << "osal_mq_close() failed";
}

} // namespace test_interrupts

} // namespace test_mqsignals

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  if (getenv("VERBOSE")) {
    test_mqsignals::verbose = 1;
  }

  return RUN_ALL_TESTS();
}
