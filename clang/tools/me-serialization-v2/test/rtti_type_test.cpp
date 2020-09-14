#include "base_derive.s11n.h"
#include <gtest/gtest.h>
#include <memory>
class RTTITypeSerializeTest : public testing::Test {};
static uint8_t buffer[64];
TEST(RTTITypeSerializeTest, rtti_type_test) {
  std::unique_ptr<Base> base, unknown;
  base = std::make_unique<Base>();
  me::serialization::WriteRaw(base, buffer);
  me::serialization::ReadRaw(unknown, buffer);
  ASSERT_EQ(unknown->Test(), "Base");
  base = std::make_unique<DerivedOne>();
  me::serialization::WriteRaw(base, buffer);
  me::serialization::ReadRaw(unknown, buffer);
  ASSERT_EQ(unknown->Test(), base->Test());
  base = std::make_unique<DerivedTwo>();
  me::serialization::WriteRaw(base, buffer);
  me::serialization::ReadRaw(unknown, buffer);
  ASSERT_EQ(unknown->Test(), "DerivedTwo");
}