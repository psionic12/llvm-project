#include "../serialization/built_in_type_coder.h"
#include "../serialization/field_coder.h"
#include <cstring>
#include <gtest/gtest.h>
class BuiltInTypeSerializeTest : public testing::Test {};
uint8_t buffer[128];
TEST_F(BuiltInTypeSerializeTest, int_test) {
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
  std::vector<unsigned long> vul{9, 10, 11, 12};
  ptr = me::serialization::Coder<decltype(vul)>::Write(vul, ptr);
  std::vector<double> vd{0.9, 0.10, 1.1, 12.};
  ptr = me::serialization::Coder<decltype(vd)>::Write(vd, ptr);
  std::vector<uint32_t> v_uint32_t{1, 2, 3, 4, 1000};
  ptr = me::serialization::
      Coder<std::vector<me::serialization::strong_uint32>>::Write(
          *(reinterpret_cast<std::vector<me::serialization::strong_uint32> *>(
              &v_uint32_t)),
          ptr);

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
  std::vector<double> vd2;
  cptr = me::serialization::Coder<decltype(vd2)>::Read(vd2, cptr);
  EXPECT_TRUE(vd == vd2);
  std::vector<uint32_t> v2_uint32_t;
  cptr = me::serialization::
      Coder<std::vector<me::serialization::strong_uint32>>::Read(
          *(reinterpret_cast<std::vector<me::serialization::strong_uint32> *>(
              &v2_uint32_t)),
          cptr);
  EXPECT_TRUE(v_uint32_t == v2_uint32_t);
}

TEST_F(BuiltInTypeSerializeTest, constexpr_test) {
  constexpr int cstxpr = 10086;
  int non_cstxpr = cstxpr;
  int non_cstxpr_size =
      me::serialization::Log2FloorHelper::Log2Floor(non_cstxpr);
  constexpr int cstxpr_size =
      me::serialization::Log2FloorHelper::Log2FloorConstexpr(cstxpr);
  ASSERT_EQ(non_cstxpr_size, cstxpr_size);
}

TEST_F(BuiltInTypeSerializeTest, size_test) {
  unsigned int i = 65535;
  auto *ptr = me::serialization::Coder<decltype(i)>::Write(i, buffer);
  ASSERT_EQ(ptr - buffer, me::serialization::Coder<decltype(i)>::Size(i));

  int j = -1000;
  ptr = me::serialization::Coder<decltype(j)>::Write(j, buffer);
  ASSERT_EQ(ptr - buffer, me::serialization::Coder<decltype(j)>::Size(j));

  float f = 3.14;
  ptr = me::serialization::Coder<decltype(f)>::Write(f, buffer);
  ASSERT_EQ(ptr - buffer, me::serialization::Coder<decltype(f)>::Size());

  double d = 2.71828;
  ptr = me::serialization::Coder<decltype(d)>::Write(d, buffer);
  ASSERT_EQ(ptr - buffer, me::serialization::Coder<decltype(d)>::Size());

  char hello[] = "hello, world!";
  ptr = me::serialization::Coder<decltype(hello)>::Write(hello, buffer);
  ASSERT_EQ(ptr - buffer, me::serialization::Coder<decltype(hello)>::Size());

  std::vector<int> v = {1, 2, 3, 4, 1000};
  ptr = me::serialization::Coder<decltype(v)>::Write(v, buffer);
  ASSERT_EQ(ptr - buffer, me::serialization::Coder<decltype(v)>::Size(v));

  int array[] = {1, 2, 3, 4, 1000};
  ptr = me::serialization::Coder<decltype(array)>::Write(array, buffer);
  ASSERT_EQ(ptr - buffer,
            me::serialization::Coder<decltype(array)>::Size(array));

  float farray[] = {1.0, 0.2, 3.14, 1000};
  ptr = me::serialization::Coder<decltype(farray)>::Write(farray, buffer);
  ASSERT_EQ(ptr - buffer, me::serialization::Coder<decltype(farray)>::Size());
}

struct Foo {
  template <typename T, typename Enable> friend struct me::serialization::Coder;

private:
  int f1 = -1;
  unsigned int f2 = 2;
  float f3 = 0.3f;
  double f4 = 0.4;
  uint8_t f5 = 5;
  int16_t f6 = 6;
  uint32_t f7 = 7;
  int64_t f8 = 8;
  int f9[4] = {9, 99, 999, 9999};
  int32_t f10[4] = {10, 100, 1000, 10000};
  std::vector<unsigned int> f11 = {11, 111, 1111, 11111};
  std::vector<uint32_t> f12 = {12, 1212, 121212, 12121212};
  std::string f13 = "f13";
};

namespace me {
namespace serialization {

template <> struct Coder<Foo> {
  static uint8_t *Write(const Foo value, uint8_t *ptr) {
    if (value.f1 != 0) {
      ptr = Coder<uint32_t>::Write(1, ptr);
      constexpr uint8_t tag =
          MakeTag(false, GraininessWrapper<decltype(value.f1)>::type);
      ptr = Coder<uint8_t>::Write(tag, ptr);
      ptr = Coder<decltype(value.f1)>::Write(value.f1, ptr);
    }
    if (value.f2 != 0) {
      ptr = Coder<uint32_t>::Write(2, ptr);
      constexpr uint8_t tag =
          MakeTag(false, GraininessWrapper<decltype(value.f2)>::type);
      ptr = Coder<uint8_t>::Write(tag, ptr);
      ptr = Coder<decltype(value.f2)>::Write(value.f2, ptr);
    }
    if (value.f3 < 0 || value.f3 > 0) {
      ptr = Coder<uint32_t>::Write(3, ptr);
      constexpr uint8_t tag =
          MakeTag(false, GraininessWrapper<decltype(value.f3)>::type);
      ptr = Coder<uint8_t>::Write(tag, ptr);
      ptr = Coder<decltype(value.f3)>::Write(value.f3, ptr);
    }
    if (value.f4 < 0 || value.f4 > 0) {
      constexpr uint8_t tag =
          MakeTag(false, GraininessWrapper<decltype(value.f4)>::type);
      ptr = Coder<uint8_t>::Write(tag, ptr);
      ptr = Coder<decltype(value.f4)>::Write(value.f4, ptr);
    }
    if (value.f5 != 0) {
      constexpr uint8_t tag =
          MakeTag(5, GraininessWrapper<decltype(value.f5)>::type);
      ptr = Coder<uint8_t>::Write(tag, ptr);
      ptr = Coder<decltype(value.f5)>::Write(value.f5, ptr);
    }
    if (value.f6 != 0) {
      constexpr uint8_t tag =
          MakeTag(6, GraininessWrapper<decltype(value.f6)>::type);
      ptr = Coder<uint8_t>::Write(tag, ptr);
      ptr = Coder<decltype(value.f6)>::Write(value.f6, ptr);
    }
    if (value.f7 != 0) {
      constexpr uint8_t tag =
          MakeTag(7, GraininessWrapper<decltype(value.f7)>::type);
      ptr = Coder<uint8_t>::Write(tag, ptr);
      ptr = Coder<decltype(value.f7)>::Write(value.f7, ptr);
    }
    if (value.f8 != 0) {
      constexpr uint8_t tag =
          MakeTag(8, GraininessWrapper<decltype(value.f8)>::type);
      ptr = Coder<uint8_t>::Write(tag, ptr);
      ptr = Coder<decltype(value.f8)>::Write(value.f8, ptr);
    }
    if (value.f9 != 0) {
      constexpr uint8_t tag =
          MakeTag(9, GraininessWrapper<decltype(value.f9)>::type);
      ptr = Coder<uint8_t>::Write(tag, ptr);
      ptr = Coder<decltype(value.f9)>::Write(value.f9, ptr);
    }

    return ptr;
  }
  static const uint8_t *Read(Foo &out, const uint8_t *ptr) {
    return nullptr;
  }
  static std::size_t Size(const Foo value) { return 0; }
};
} // namespace serialization
} // namespace me

TEST_F(BuiltInTypeSerializeTest, class_simulation) {}
