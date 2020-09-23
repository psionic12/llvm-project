#include "foo.s11n.h"
#include <gtest/gtest.h>
class CustomTypeS11nTest : public testing::Test {};
static uint8_t buffer[256];
TEST_F(CustomTypeS11nTest, class_simulation) {
  Foo foo;
  auto *ptr = me::s11n::WriteRaw(foo, buffer);
  ASSERT_EQ(ptr - buffer, me::s11n::SizeRaw(foo));
  Foo foo2;
  memset(&foo2, 0, sizeof(foo2));
  me::s11n::ReadRaw(foo2, buffer);
  ASSERT_EQ(foo.f1, foo2.f1);
  ASSERT_EQ(foo.f2, foo2.f2);
  ASSERT_EQ(foo.f3, foo2.f3);
  ASSERT_EQ(foo.f4, foo2.f4);
  ASSERT_EQ(foo.f5, foo2.f5);
  ASSERT_EQ(foo.f6, foo2.f6);
  ASSERT_EQ(foo.f7, foo2.f7);
  ASSERT_EQ(foo.f8, foo2.f8);
  ASSERT_TRUE(memcmp(foo.f9, foo2.f9, sizeof(foo.f9)) == 0);
  ASSERT_TRUE(memcmp(foo.f10, foo2.f10, sizeof(foo.f10)) == 0);
  ASSERT_EQ(foo.f11, foo2.f11);
  ASSERT_EQ(foo.f12, foo2.f12);
  ASSERT_EQ(foo.f13, foo2.f13);
  ASSERT_TRUE(memcmp(foo.f14, foo2.f14, sizeof(foo.f14)) == 0);
//  ASSERT_EQ(*foo.f16, *foo2.f16);
}