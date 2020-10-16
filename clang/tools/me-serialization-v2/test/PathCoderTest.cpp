#include "../src/common/PathHelper.h"
#include <gtest/gtest.h>
class PathCoderTest : public testing::Test {};

TEST_F(PathCoderTest, convert_test) {
  std::string Path = "path/to/demo.h";
  std::string Converted = "path$to$demo.h";
  ASSERT_EQ(convertPathToSingleName(Path), Converted);
  ASSERT_EQ(convertSingleNameToPath(Converted), Path);

  Path = "path/to/demo$1.h";
  Converted = "path$to$demo$$1.h";
  ASSERT_EQ(convertPathToSingleName(Path), Converted);
  ASSERT_EQ(convertSingleNameToPath(Converted), Path);
}

TEST_F(PathCoderTest, relative_test) {
  std::string Base = "base1/base2/";
  std::string Path = "base1/base2/child/demo.h";
  ASSERT_EQ(relative(Base, Path), "child/demo.h");

  Base = "base1/base2";
  Path = "base1/base2/child/demo.h";
  ASSERT_EQ(relative(Base, Path), "child/demo.h");

  Base = "base1/base2";
  Path = "base1/base22/child/demo.h";
  ASSERT_EQ(relative(Base, Path), "base22/child/demo.h");

  Base = "base1/base2";
  Path = "base3/base4/child/demo.h";
  ASSERT_EQ(relative(Base, Path), "");
}