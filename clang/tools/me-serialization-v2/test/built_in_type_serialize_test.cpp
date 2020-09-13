#include "../serialization/built_in_type_coder.h"
#include "../serialization/field_coder.h"
#include <cstring>
#include <gtest/gtest.h>
class BuiltInTypeSerializeTest : public testing::Test {};
uint8_t buffer[256];
TEST_F(BuiltInTypeSerializeTest, int_test) {
  uint8_t *ptr = buffer;
  int i = -42;
  ptr = me::serialization::WriteRaw(i, ptr);
  float f = 3.14f;
  ptr = me::serialization::WriteRaw(f, ptr);
  unsigned int ui = 42;
  ptr = me::serialization::WriteRaw(ui, ptr);
  int array[] = {1, 2, 3, 4};
  ptr = me::serialization::WriteRaw(array, ptr);
  char c[] = "abcd";
  ptr = me::serialization::WriteRaw(c, ptr);
  std::string s = "efg";
  ptr = me::serialization::WriteRaw(s, ptr);
  std::vector<int> vi{5, 6, 7, 8};
  ptr = me::serialization::WriteRaw(vi, ptr);
  std::vector<unsigned long> vul{9, 10, 11, 12};
  ptr = me::serialization::WriteRaw(vul, ptr);
  std::vector<double> vd{0.9, 0.10, 1.1, 12.};
  ptr = me::serialization::WriteRaw(vd, ptr);
  std::vector<uint32_t> v_uint32_t{1, 2, 3, 4, 1000};
  ptr = me::serialization::WriteRaw(
      *(reinterpret_cast<std::vector<me::serialization::strong_uint32> *>(
          &v_uint32_t)),
      ptr);

  const uint8_t *cptr = buffer;
  int i2;
  cptr = me::serialization::ReadRaw(i2, cptr);
  ASSERT_EQ(i, i2);
  float f2;
  cptr = me::serialization::ReadRaw(f2, cptr);
  ASSERT_EQ(f, f2);
  unsigned int ui2;
  cptr = me::serialization::ReadRaw(ui2, cptr);
  ASSERT_EQ(ui, ui2);
  int array2[4];
  cptr = me::serialization::ReadRaw(array2, cptr);
  EXPECT_TRUE(0 == std::memcmp(array, array2, sizeof(array)));
  char c2[5];
  cptr = me::serialization::ReadRaw(c2, cptr);
  EXPECT_TRUE(0 == std::memcmp(c, c2, sizeof(c)));
  std::string s2;
  cptr = me::serialization::ReadRaw(s2, cptr);
  EXPECT_TRUE(s == s2);
  std::vector<int> vi2;
  cptr = me::serialization::ReadRaw(vi2, cptr);
  EXPECT_TRUE(vi == vi2);
  std::vector<unsigned long> vul2;
  cptr = me::serialization::ReadRaw(vul2, cptr);
  EXPECT_TRUE(vul == vul2);
  std::vector<double> vd2;
  cptr = me::serialization::ReadRaw(vd2, cptr);
  EXPECT_TRUE(vd == vd2);
  std::vector<uint32_t> v2_uint32_t;
  cptr = me::serialization::ReadRaw(
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
  auto *ptr = me::serialization::WriteRaw(i, buffer);
  ASSERT_EQ(ptr - buffer, me::serialization::SizeRaw(i));

  int j = -1000;
  ptr = me::serialization::WriteRaw(j, buffer);
  ASSERT_EQ(ptr - buffer, me::serialization::SizeRaw(j));

  float f = 3.14;
  ptr = me::serialization::WriteRaw(f, buffer);
  ASSERT_EQ(ptr - buffer, me::serialization::SizeRaw(f));

  double d = 2.71828;
  ptr = me::serialization::WriteRaw(d, buffer);
  ASSERT_EQ(ptr - buffer, me::serialization::SizeRaw(d));

  char hello[] = "hello, world!";
  ptr = me::serialization::WriteRaw(hello, buffer);
  ASSERT_EQ(ptr - buffer, me::serialization::SizeRaw(hello));

  std::vector<int> v = {1, 2, 3, 4, 1000};
  ptr = me::serialization::WriteRaw(v, buffer);
  ASSERT_EQ(ptr - buffer, me::serialization::SizeRaw(v));

  int array[] = {1, 2, 3, 4, 1000};
  ptr = me::serialization::WriteRaw(array, buffer);
  ASSERT_EQ(ptr - buffer, me::serialization::SizeRaw(array));

  float farray[] = {1.0, 0.2, 3.14, 1000};
  ptr = me::serialization::WriteRaw(farray, buffer);
  ASSERT_EQ(ptr - buffer, me::serialization::SizeRaw(farray));
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
  float f14[4] = {1.0, 0.1, 0.01, 0.001};
  std::vector<unsigned int> f11 = {11, 111, 1111, 11111};
  std::vector<uint32_t> f12 = {12, 1212, 121212, 12121212};
  std::string f13 = "f13";
};

namespace me {
namespace serialization {

std::unordered_map<std::size_t, std::unique_ptr<CoderWrapper>>
    CoderWrapper::IdToCoderMap;
std::unordered_map<std::type_index, std::size_t> CoderWrapper::CppIdToIdMap;

template <> struct Coder<Foo> {
  static uint8_t *Write(const Foo value, uint8_t *ptr) {
    ptr = WriteRaw(Size(value), ptr);
    ptr = WriteField(1, value.f1, ptr);
    ptr = WriteField(2, value.f2, ptr);
    ptr = WriteField(3, value.f3, ptr);
    ptr = WriteField(4, value.f4, ptr);
    ptr = WriteField(5, value.f5, ptr);
    ptr = WriteField(6, value.f6, ptr);
    ptr = WriteField(7, value.f7, ptr);
    ptr = WriteField(8, value.f8, ptr);
    ptr = WriteField(9, value.f9, ptr);
    ptr = WriteField(10, value.f10, ptr);
    ptr = WriteField(11, value.f11, ptr);
    ptr = WriteField(12, value.f12, ptr);
    ptr = WriteField(13, value.f13, ptr);
    ptr = WriteField(14, value.f14, ptr);
    return ptr;
  }
  static const uint8_t *Read(Foo &out, const uint8_t *ptr) {
    std::size_t total_size;
    ptr = ReadRaw(total_size, ptr);
    const uint8_t *ptr_end = ptr + total_size;
    while (ptr < ptr_end) {
      uint32_t index;
      ptr = ReadRaw(index, ptr);
      uint8_t tag;
      ptr = ReadRaw(tag, ptr);
      switch (index) {
      case 0:
        // TODO should not have 0 index
        break;
      case 1:
        // TODO unlikely tag check
        ptr = ReadRaw(out.f1, ptr);
        break;
      case 2:
        ptr = ReadRaw(out.f2, ptr);
        break;
      case 3:
        ptr = ReadRaw(out.f3, ptr);
        break;
      case 4:
        ptr = ReadRaw(out.f4, ptr);
        break;
      case 5:
        ptr = ReadRaw(out.f5, ptr);
        break;
      case 6:
        ptr = ReadRaw(out.f6, ptr);
        break;
      case 7:
        ptr = ReadRaw(out.f7, ptr);
        break;
      case 8:
        ptr = ReadRaw(out.f8, ptr);
        break;
      case 9:
        ptr = ReadRaw(out.f9, ptr);
        break;
      case 10:
        ptr = ReadRaw(out.f10, ptr);
        break;
      case 11:
        ptr = ReadRaw(out.f11, ptr);
        break;
      case 12:
        ptr = ReadRaw(out.f12, ptr);
        break;
      case 13:
        ptr = ReadRaw(out.f13, ptr);
        break;
      case 14:
        ptr = ReadRaw(out.f14, ptr);
        break;
      default: {
        if (HasRtti(tag)) {
          ptr = SkipVarint(ptr);
        }
        switch (GetGraininess(tag)) {
        case Graininess::BIT_8: {
          ptr += 1;
          break;
        }
        case Graininess::BIT_16: {
          ptr += 2;
          break;
        }
        case Graininess::BIT_32: {
          ptr += 4;
          break;
        }
        case Graininess::BIT_64: {
          ptr += 8;
          break;
        }
        case Graininess::VARINT: {
          ptr = SkipVarint(ptr);
        }
        case Graininess::LENGTH_DELIMITED: {
          uint64_t size;
          ptr = ReadRaw(size, ptr);
          ptr += size;
          break;
        }
        }
      }
      }
    }
    return ptr;
  }
  static std::size_t Size(const Foo value) {
    std::size_t size = 0;
    size += FieldSize<1>(value.f1);
    size += FieldSize<2>(value.f2);
    size += FieldSize<3>(value.f3);
    size += FieldSize<4>(value.f4);
    size += FieldSize<5>(value.f5);
    size += FieldSize<6>(value.f6);
    size += FieldSize<7>(value.f7);
    size += FieldSize<8>(value.f8);
    size += FieldSize<9>(value.f9);
    size += FieldSize<10>(value.f10);
    size += FieldSize<11>(value.f11);
    size += FieldSize<14>(value.f14);
    return size;
  }
};
} // namespace serialization
} // namespace me

TEST_F(BuiltInTypeSerializeTest, class_simulation) {
  Foo foo;
  //  ASSERT_EQ(me::serialization::SizeRaw(foo), 0);
  me::serialization::WriteRaw(foo, buffer);
  Foo foo2;
  memset(&foo2, 0, sizeof(foo2));
  me::serialization::ReadRaw(foo2, buffer);
  ASSERT_TRUE(memcmp(&foo, &foo2, sizeof(foo)) == 0);
}
