#include "base_derive.s11n.h"
#include <gtest/gtest.h>
#include <memory>
class RTTITypeSerializeTest : public testing::Test {};
static uint8_t buffer[64];
TEST(RTTITypeSerializeTest, rtti_type_test) {
  std::unique_ptr<Base> base, test;
  base = std::make_unique<Base>();
  me::serialization::WriteRaw(base, buffer);
  me::serialization::ReadRaw(test, buffer);
  ASSERT_EQ(test->Test(), "Base");
}