#include "gtest/gtest.h"
#include <pthread.h>
#include <vector>

#include "libosal/mq.h"
#include "libosal/osal.h"
#include "test_utils.h"

namespace test_messagequeue {

/* These tests are essentially a replica of the functional ones in
   test_messagequeue.cc, but with the difference that the timed send
   and receive functions are used.
*/

extern int verbose;

const ulong NUM_MESSAGES = 100;
const ulong TIMEOUT_NS = 10000000;
const ulong DELAY_TIME_SEC = 1;
const ulong NUM_DELAYS_MIN_EXPECT = 100;

typedef struct {
  ulong payload;
} message_t;

typedef struct {
  uint send_wait_count;
  uint recv_wait_count;
  osal_mq_t queue;
} shared_t;

void *run_producer(void *p_params) {

  shared_t *pshared = ((shared_t *)p_params);

  osal_retval_t orv;

  message_t msg;
  if (verbose) {
    printf("started: producer\n");
  }

  for (ulong i = 0; i < NUM_MESSAGES; i++) {
    // draw a random value as payload
    msg.payload = i;
    osal_uint32_t const prio = 0;

    timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    now.tv_nsec += TIMEOUT_NS;
    if (now.tv_nsec > 1000000000) {
      now.tv_nsec -= 1000000000;
      now.tv_sec += 1;
    }
    osal_timer_t deadline;
    deadline.sec = now.tv_sec;
    deadline.nsec = now.tv_nsec;

    while (true) {
      orv = osal_mq_timedsend(&pshared->queue, (char *)&msg, sizeof(msg), prio,
                              &deadline);
      if (orv == OSAL_ERR_TIMEOUT) {
        pshared->send_wait_count++;
        continue;
      }
      EXPECT_EQ(orv, OSAL_OK) << "osal_mq_send() failed";
      break;
    }
  }

  if (verbose) {
    printf("exiting: producer\n");
  }
  return nullptr;
}

void *run_consumer(void *p_params) {

  shared_t *pshared = ((shared_t *)p_params);

  osal_retval_t orv;

  message_t msg;

  if (verbose) {
    printf("started: consumer\n");
  }

  for (ulong i = 0; i < NUM_MESSAGES; i++) {

    osal_uint32_t rprio = 0;

    timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    now.tv_nsec += TIMEOUT_NS;
    if (now.tv_nsec > 1000000000) {
      now.tv_nsec -= 1000000000;
      now.tv_sec += 1;
    }
    osal_timer_t deadline;
    deadline.sec = now.tv_sec;
    deadline.nsec = now.tv_nsec;

    while (true) {
      orv = osal_mq_timedreceive(&pshared->queue, (char *)&msg, sizeof(msg),
                                 &rprio, &deadline);
      if (orv == OSAL_ERR_TIMEOUT) {
        pshared->recv_wait_count++;
        continue;
      }

      EXPECT_EQ(orv, OSAL_OK) << "osal_mq_receive() failed";
      break;
    }
  }

  if (verbose) {
    printf("exiting: consumer\n");
  }
  return nullptr;
}

TEST(MessageQueueFunction, TimeoutsDelayedSend) {

  int rv;
  osal_retval_t orv;

  shared_t shared;

  pthread_t producer;
  pthread_t consumer;

  // initialize counters
  shared.send_wait_count = 0;
  shared.recv_wait_count = 0;

  // initialize message queue
  osal_mq_attr_t attr = {};
  attr.oflags = OSAL_MQ_ATTR__OFLAG__RDWR | OSAL_MQ_ATTR__OFLAG__CREAT;
  attr.max_messages = 10; /* system default for OSL15.4 . Depending on
                           * the platform, this won't work with a larger
                           * number without adjustment. */
  ASSERT_GE(attr.max_messages, 0u);
  attr.max_message_size = sizeof(message_t);
  ASSERT_GE(attr.max_message_size, 0u);
  attr.mode = S_IRUSR | S_IWUSR;
  // unlink message queue if it exists already.
  // Note: the return value is intentionally not checked.
  mq_unlink("/test2");

  orv = osal_mq_open(&shared.queue, "/test1", &attr);
  if (orv != 0) {
    perror("failed to open mq:");
  }
  ASSERT_EQ(orv, OSAL_OK) << "osal_mq_open() failed";

  // initialize producers
  if (verbose) {
    printf("starting producer\n");
  }

  rv = pthread_create(/*thread*/ &(producer),
                      /*pthread_attr*/ nullptr,
                      /* start_routine */ run_producer,
                      /* arg */ (void *)(&shared));
  ASSERT_EQ(rv, 0) << "pthread_create()[producer] failed";

  // introduce delay
  sleep(DELAY_TIME_SEC);

  // initialize consumer
  if (verbose) {
    printf("starting consumer\n");
  }

  rv = pthread_create(/*thread*/ &(consumer),
                      /*pthread_attr*/ nullptr,
                      /* start_routine */ run_consumer,
                      /* arg */ (void *)(&shared));
  ASSERT_EQ(rv, 0) << "pthread_create()[consumer] failed";

  if (verbose) {
    printf("joining producer\n");
  }

  rv = pthread_join(/*thread*/ producer,
                    /*retval*/ nullptr);
  ASSERT_EQ(rv, 0) << "pthread_join()[producer] failed";

  // join consumer
  if (verbose) {
    printf("joining consumer\n");
  }

  rv = pthread_join(/*thread*/ consumer,
                    /*retval*/ nullptr);
  ASSERT_EQ(rv, 0) << "pthread_join()[consumers] failed";

  // destroy message queue
  orv = osal_mq_close(&shared.queue);
  ASSERT_EQ(orv, OSAL_OK) << "osal_mq_close() failed";

  // compare results for correctness

  EXPECT_GE(shared.send_wait_count, NUM_DELAYS_MIN_EXPECT)
      << "send_waitcount too small";
  EXPECT_GE(shared.recv_wait_count, 0u)
      << "wait count 0 expected for receiving";
}

TEST(MessageQueueFunction, TimeoutsDelayedRecv) {

  int rv;
  osal_retval_t orv;

  shared_t shared;

  pthread_t producer;
  pthread_t consumer;

  // initialize counters
  shared.send_wait_count = 0;
  shared.recv_wait_count = 0;

  // initialize message queue
  osal_mq_attr_t attr = {};
  attr.oflags = OSAL_MQ_ATTR__OFLAG__RDWR | OSAL_MQ_ATTR__OFLAG__CREAT;
  attr.max_messages = 10; /* system default, won't work with larger
                           * number without adjustment */
  ASSERT_GE(attr.max_messages, 0u);
  attr.max_message_size = sizeof(message_t);
  ASSERT_GE(attr.max_message_size, 0u);
  attr.mode = S_IRUSR | S_IWUSR;
  // unlink message queue if it exists.
  // Note: the return value is intentionally not checked.
  mq_unlink("/test2");

  orv = osal_mq_open(&shared.queue, "/test1", &attr);
  if (orv != 0) {
    perror("failed to open mq:");
  }
  ASSERT_EQ(orv, OSAL_OK) << "osal_mq_open() failed";

  // Compared to above, sender and producer are swapped.
  // Initialize consumer.
  if (verbose) {
    printf("starting consumer\n");
  }

  rv = pthread_create(/*thread*/ &(consumer),
                      /*pthread_attr*/ nullptr,
                      /* start_routine */ run_consumer,
                      /* arg */ (void *)(&shared));
  ASSERT_EQ(rv, 0) << "pthread_create()[consumer] failed";
  // introduce delay
  sleep(DELAY_TIME_SEC);

  // initialize producers
  if (verbose) {
    printf("starting producer\n");
  }

  rv = pthread_create(/*thread*/ &(producer),
                      /*pthread_attr*/ nullptr,
                      /* start_routine */ run_producer,
                      /* arg */ (void *)(&shared));
  ASSERT_EQ(rv, 0) << "pthread_create()[producer] failed";

  if (verbose) {
    printf("joining producer\n");
  }

  rv = pthread_join(/*thread*/ producer,
                    /*retval*/ nullptr);
  ASSERT_EQ(rv, 0) << "pthread_join()[producer] failed";

  // join consumer
  if (verbose) {
    printf("joining consumer\n");
  }

  rv = pthread_join(/*thread*/ consumer,
                    /*retval*/ nullptr);
  ASSERT_EQ(rv, 0) << "pthread_join()[consumers] failed";

  // destroy message queue
  orv = osal_mq_close(&shared.queue);
  ASSERT_EQ(orv, OSAL_OK) << "osal_mq_close() failed";

  // compare results for correctness

  EXPECT_GE(shared.send_wait_count, 0u) << "wait count 0 expected for sending";
  EXPECT_GE(shared.recv_wait_count, NUM_DELAYS_MIN_EXPECT)
      << "send_waitcount too small";
}

} // namespace test_messagequeue
