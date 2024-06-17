#include "gtest/gtest.h"

#include "libosal/io.h"
#include "libosal/osal.h"
#include <unistd.h>

namespace test_shmio {

TEST(SHMIOFunction, SimpleMessage) {
  const char TEST_MESSAGE[] = "abcd\n";

  unlink("/dev/shm/shm_io");
  osal_retval_t orv = osal_io_shm_setup("shm_io", 1024, 512);
  ASSERT_EQ(orv, 0) << " setting up shm io failed";

  orv = osal_printf(TEST_MESSAGE);

  osal_char_t msg[LIBOSAL_IO_SHM_MAX_MSG_SIZE];
  osal_timer_t deadline = {(osal_uint64_t)time(nullptr) + 2, 0};

  orv = osal_io_shm_get_message(msg, &deadline);

  EXPECT_EQ(orv, 0) << " osal_io_shm_get_message failed";

  int ret = strcmp(msg, TEST_MESSAGE);
  EXPECT_EQ(ret, 0) << "test strings did not turn out equal: '" << msg
                    << "' vs. '" << TEST_MESSAGE << "'";
}

} // namespace test_shmio

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
