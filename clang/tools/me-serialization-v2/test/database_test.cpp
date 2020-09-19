#include <../database/record_database.h>
#include <gtest/gtest.h>
class DatabaseTest : public testing::Test {};

TEST_F(DatabaseTest, parser_test) {
RecordDatabase database;
database.parse("example_record_database.txt");
}