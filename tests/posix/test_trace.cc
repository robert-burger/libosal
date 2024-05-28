#include "gtest/gtest.h"
#include <pthread.h>
#include <vector>

#include "libosal/osal.h"
#include "libosal/trace.h"
#include "test_utils.h"

namespace test_trace {

using testutils::wait_nanoseconds;

TEST(Trace, SingleThreaded) {
  const osal_uint32_t count = 5000;
  const osal_uint32_t trsize = 2 * count;
  osal_retval_t orv;
  osal_trace_t *tracep;

  orv = osal_trace_alloc(&tracep, trsize);
  EXPECT_EQ(orv, OSAL_OK) << "osal_trace_alloc() failed";

  for (uint32_t i = 0; i < count; i++) {
    orv = osal_trace_point(tracep);

    wait_nanoseconds(1000);
    osal_uint64_t now = osal_timer_gettime_nsec();

    osal_trace_time(tracep, now);
    wait_nanoseconds(1000);

    osal_uint64_t last = osal_trace_get_last_time(tracep);
    EXPECT_EQ(now, last) << "trace_time should be equal";
  }

  osal_uint64_t avg;
  osal_uint64_t avg_jit;
  osal_uint64_t max_jit;
  osal_trace_analyze(tracep, &avg, &avg_jit, &max_jit);

  printf("trace average times: %lu, jitter: %lu, max jitter: %lu\n", avg,
         avg_jit, max_jit);

  osal_trace_analyze_rel(tracep, &avg, &avg_jit, &max_jit);
  printf("trace average rel times: %lu, jitter: %lu, max jitter: %lu\n", avg,
         avg_jit, max_jit);

  osal_trace_free(tracep);
}

} // namespace test_trace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
