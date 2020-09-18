#include "me/s11n/built_in_type_coder.h"
#include <cstring>
#include <gtest/gtest.h>
class BuiltInTypeSerializeTest : public testing::Test {};
static uint8_t buffer[256];
TEST_F(BuiltInTypeSerializeTest, int_test) {
  uint8_t *ptr = buffer;
  int i = -42;
  ptr = me::s11n::WriteRaw(i, ptr);
  float f = 3.14f;
  ptr = me::s11n::WriteRaw(f, ptr);
  unsigned int ui = 42;
  ptr = me::s11n::WriteRaw(ui, ptr);
  int array[] = {1, 2, 3, 4};
  ptr = me::s11n::WriteRaw(array, ptr);
  char c[] = "abcd";
  ptr = me::s11n::WriteRaw(c, ptr);
  std::string s = "efg";
  ptr = me::s11n::WriteRaw(s, ptr);
  std::vector<int> vi{5, 6, 7, 8};
  ptr = me::s11n::WriteRaw(vi, ptr);
  std::vector<unsigned long> vul{9, 10, 11, 12};
  ptr = me::s11n::WriteRaw(vul, ptr);
  std::vector<double> vd{0.9, 0.10, 1.1, 12.};
  ptr = me::s11n::WriteRaw(vd, ptr);
  std::vector<uint32_t> v_uint32_t{1, 2, 3, 4, 1000};
  ptr = me::s11n::WriteRaw(
      *(reinterpret_cast<std::vector<me::s11n::strong_uint32> *>(
          &v_uint32_t)),
      ptr);

  const uint8_t *cptr = buffer;
  int i2;
  cptr = me::s11n::ReadRaw(i2, cptr);
  ASSERT_EQ(i, i2);
  float f2;
  cptr = me::s11n::ReadRaw(f2, cptr);
  ASSERT_EQ(f, f2);
  unsigned int ui2;
  cptr = me::s11n::ReadRaw(ui2, cptr);
  ASSERT_EQ(ui, ui2);
  int array2[4];
  cptr = me::s11n::ReadRaw(array2, cptr);
  EXPECT_TRUE(0 == std::memcmp(array, array2, sizeof(array)));
  char c2[5];
  cptr = me::s11n::ReadRaw(c2, cptr);
  EXPECT_TRUE(0 == std::memcmp(c, c2, sizeof(c)));
  std::string s2;
  cptr = me::s11n::ReadRaw(s2, cptr);
  EXPECT_TRUE(s == s2);
  std::vector<int> vi2;
  cptr = me::s11n::ReadRaw(vi2, cptr);
  EXPECT_TRUE(vi == vi2);
  std::vector<unsigned long> vul2;
  cptr = me::s11n::ReadRaw(vul2, cptr);
  EXPECT_TRUE(vul == vul2);
  std::vector<double> vd2;
  cptr = me::s11n::ReadRaw(vd2, cptr);
  EXPECT_TRUE(vd == vd2);
  std::vector<uint32_t> v2_uint32_t;
  cptr = me::s11n::ReadRaw(
      *(reinterpret_cast<std::vector<me::s11n::strong_uint32> *>(
          &v2_uint32_t)),
      cptr);
  EXPECT_TRUE(v_uint32_t == v2_uint32_t);
}

TEST_F(BuiltInTypeSerializeTest, constexpr_test) {
  constexpr int cstxpr = 10086;
  int non_cstxpr = cstxpr;
  int non_cstxpr_size =
      me::s11n::Log2FloorHelper::Log2Floor(non_cstxpr);
  constexpr int cstxpr_size =
      me::s11n::Log2FloorHelper::Log2FloorConstexpr(cstxpr);
  ASSERT_EQ(non_cstxpr_size, cstxpr_size);
}

TEST_F(BuiltInTypeSerializeTest, size_test) {
  unsigned int i = 65535;
  auto *ptr = me::s11n::WriteRaw(i, buffer);
  ASSERT_EQ(ptr - buffer, me::s11n::SizeRaw(i));

  int j = -1000;
  ptr = me::s11n::WriteRaw(j, buffer);
  ASSERT_EQ(ptr - buffer, me::s11n::SizeRaw(j));

  float f = 3.14;
  ptr = me::s11n::WriteRaw(f, buffer);
  ASSERT_EQ(ptr - buffer, me::s11n::SizeRaw(f));

  double d = 2.71828;
  ptr = me::s11n::WriteRaw(d, buffer);
  ASSERT_EQ(ptr - buffer, me::s11n::SizeRaw(d));

  char hello[] = "hello, world!";
  ptr = me::s11n::WriteRaw(hello, buffer);
  ASSERT_EQ(ptr - buffer, me::s11n::SizeRaw(hello));

  std::vector<int> v = {1, 2, 3, 4, 1000};
  ptr = me::s11n::WriteRaw(v, buffer);
  ASSERT_EQ(ptr - buffer, me::s11n::SizeRaw(v));

  int array[] = {1, 2, 3, 4, 1000};
  ptr = me::s11n::WriteRaw(array, buffer);
  ASSERT_EQ(ptr - buffer, me::s11n::SizeRaw(array));

  float farray[] = {1.0, 0.2, 3.14, 1000};
  ptr = me::s11n::WriteRaw(farray, buffer);
  ASSERT_EQ(ptr - buffer, me::s11n::SizeRaw(farray));
}
