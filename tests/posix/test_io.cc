#include "gtest/gtest.h"

#include "libosal/io.h"
#include "libosal/osal.h"

namespace test_io {

TEST(IO, PrintF) {

  osal_retval_t orv = osal_printf("Hello World!\n");

  // NOTE: return values are defined differently from POSIX printf,
  // where printf returns the number of printed characters
  EXPECT_EQ(orv, 0) << " osal_printf() did not return zero";

  orv = osal_printf("this is %i\n", 100);

#pragma message                                                                \
    "FIXME: return value of osal_printf() does not return number of bytes"

  EXPECT_EQ(orv, 0) << " osal_printf(..., 100) did not return zero";
}
TEST(IO, PutS) {

  osal_retval_t orv = osal_puts("abc");

  EXPECT_EQ(orv, 0) << " osal_puts() did not return zero";
}

osal_int32_t test_func(const char *format, const char *args, ...) {

  va_list ap;
  va_start(ap, args);
  osal_int32_t orv = osal_vfprintf(stdout, format, ap);
  va_end(ap);

  return orv;
}

TEST(IO, vfprintf) {
  osal_int32_t orv = test_func("test: %s, %s\n", "argument1", "argument2");

  EXPECT_EQ(orv, 31) << " osal_vfprintf() did not return zero";
}

} // namespace test_io

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
