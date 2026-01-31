#include <gtest/gtest.h>

#include "schema.h"
#include "../tests/utility.h"

TEST(Schema, BasicFromCsv) {
    std::string content =
        "a,int64\n"
        "b,string\n"
        ;
    std::string path = CreateTempFile(content);
    
    Schema schema = Schema::FromCsv(path);

    EXPECT_EQ(schema.Size(), 2);

    EXPECT_EQ(schema.GetColumns()[0].name, "a");
    EXPECT_EQ(schema.GetColumns()[1].name, "b");

    EXPECT_EQ(schema.GetColumns()[0].type, Type::Int64);
    EXPECT_EQ(schema.GetColumns()[1].type, Type::String);
}

TEST(Schema, FromCsvInvalidCsv) {
    std::string content =
        "a,int64,int32\n"
        ;
    std::string path = CreateTempFile(content);
    
    EXPECT_ANY_THROW(Schema::FromCsv(path));
}

TEST(Schema, FromCsvTwoEqualColumns) {
    std::string content =
        "a,int64\n"
        "a,string\n"
        ;
    std::string path = CreateTempFile(content);
    
    EXPECT_ANY_THROW(Schema::FromCsv(path));
}

TEST(Schema, AddColumnBasic) {
    std::string content = "";
    std::string path = CreateTempFile(content);
    
    Schema schema = Schema::FromCsv(path);

    EXPECT_NO_THROW(schema.AddColumn(Schema::ColumnInfo{"a", Type::Int64}));
    EXPECT_EQ(schema.Size(), 1);
    EXPECT_NO_THROW(schema.AddColumn(Schema::ColumnInfo{"b", Type::String}));
    EXPECT_EQ(schema.Size(), 2);
    EXPECT_NO_THROW(schema.AddColumn(Schema::ColumnInfo{"c", Type::String}));
    EXPECT_EQ(schema.Size(), 3);
    EXPECT_NO_THROW(schema.AddColumn(Schema::ColumnInfo{"d", Type::String}));
    EXPECT_EQ(schema.Size(), 4);
}

TEST(Schema, AddColumnTwoEqualColumns) {
    std::string content = "";
    std::string path = CreateTempFile(content);
    
    Schema schema = Schema::FromCsv(path);

    EXPECT_NO_THROW(schema.AddColumn(Schema::ColumnInfo{"a", Type::Int64}));
    EXPECT_EQ(schema.Size(), 1);
    EXPECT_ANY_THROW(schema.AddColumn(Schema::ColumnInfo{"a", Type::String}));
}