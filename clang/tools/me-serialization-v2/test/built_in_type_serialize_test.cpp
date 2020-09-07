#include <gtest/gtest.h>
#include "../serialization/built_in_type_coder.h"
class BuiltInTypeSerializeTest : public testing::Test {};

TEST_F(BuiltInTypeSerializeTest, int_test) {
  int i = 42;
  uint8_t buffer[64];
  me::serialization::Coder<decltype(i)>::Write(i, buffer);
  int j;
  me::serialization::Coder<decltype(j)>::Read(j, buffer);
  ASSERT_EQ(i, j);

  float f = 3.14f;
  me::serialization::Coder<decltype(f)>::Write(f, buffer);
  float f2;
  me::serialization::Coder<decltype(f2)>::Read(f2, buffer);
  ASSERT_EQ(f, f2);
}