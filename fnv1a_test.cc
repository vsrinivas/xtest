#include <stdint.h>
#include <sys/types.h>

#include "third_party/ninja-test/src/test.h"
#include "fnv1a.h"

TEST(Fnv1A32Test, FNV1A_32) {
  char buf[] = "hello, world";
  uint32_t val = FNV1A_32(buf, sizeof(buf));
  EXPECT_EQ(0x6b0c59a7, val);
}

TEST(Fnv1A64Test, FNV1A_64) {
  char buf[] = "hello, world";
  uint64_t val = FNV1A_64(buf, sizeof(buf));
  EXPECT_EQ(0xe60e84e64882a0a7ul, val);
}
