#include "../serialization/built_in_type_coder.h"
#include <cstring>
#include <gtest/gtest.h>
class BuiltInTypeSerializeTest : public testing::Test {};

TEST_F(BuiltInTypeSerializeTest, int_test) {
  uint8_t buffer[64];
  uint8_t *ptr = buffer;
  int i = -42;
  ptr = me::serialization::Coder<decltype(i)>::Write(i, ptr);
  float f = 3.14f;
  ptr = me::serialization::Coder<decltype(f)>::Write(f, ptr);
  unsigned int ui = 42;
  ptr = me::serialization::Coder<decltype(ui)>::Write(ui, ptr);
  int array[] = {1, 2, 3, 4};
  ptr = me::serialization::Coder<decltype(array)>::Write(array, ptr);
  char c[] = "abcd";
  ptr = me::serialization::Coder<decltype(c)>::Write(c, ptr);
  std::string s = "efg";
  ptr = me::serialization::Coder<decltype(s)>::Write(s, ptr);
  std::vector<int> vi{5, 6, 7, 8};
  ptr = me::serialization::Coder<decltype(vi)>::Write(vi, ptr);
  std::vector<unsigned long> vul {9, 10, 11, 12};
  ptr = me::serialization::Coder<decltype(vul)>::Write(vul, ptr);

  const uint8_t *cptr = buffer;
  int i2;
  cptr = me::serialization::Coder<decltype(i2)>::Read(i2, cptr);
  ASSERT_EQ(i, i2);
  float f2;
  cptr = me::serialization::Coder<decltype(f2)>::Read(f2, cptr);
  ASSERT_EQ(f, f2);
  unsigned int ui2;
  cptr = me::serialization::Coder<decltype(ui2)>::Read(ui2, cptr);
  ASSERT_EQ(ui, ui2);
  int array2[4];
  cptr = me::serialization::Coder<decltype(array2)>::Read(array2, cptr);
  EXPECT_TRUE(0 == std::memcmp(array, array2, sizeof(array)));
  char c2[5];
  cptr = me::serialization::Coder<decltype(c2)>::Read(c2, cptr);
  EXPECT_TRUE(0 == std::memcmp(c, c2, sizeof(c)));
  std::string s2;
  cptr = me::serialization::Coder<decltype(s2)>::Read(s2, cptr);
  EXPECT_TRUE(s == s2);
  std::vector<int> vi2;
  cptr = me::serialization::Coder<decltype(vi2)>::Read(vi2, cptr);
  EXPECT_TRUE(vi == vi2);
  std::vector<unsigned long> vul2;
  cptr = me::serialization::Coder<decltype(vul2)>::Read(vul2, cptr);
  EXPECT_TRUE(vul == vul2);
}