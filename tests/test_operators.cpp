#include <gtest/gtest.h>

#include "operators/Count.h"
#include "operators/Scan.h"
#include "utility.h"

TEST(Operators, Q1) {
    Schema schema({{"UserID", Type::Int64}, {"Name", Type::String}});
    auto path = MakeTempDb("test_q1.skewdb", schema, {
        {"1", "a"},
        {"2", "b"},
        {"3", "c"},
    });

    Count count(std::make_unique<Scan>(path, Schema()));
    auto result = count.Next();

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->row_count, 1);
    ASSERT_EQ(result->names.size(), 1);
    EXPECT_EQ(result->columns[0]->Get(0), Value{int64_t{3}});
    EXPECT_FALSE(count.Next().has_value());
}

TEST(Operators, ScanProjection) {
    Schema schema({{"UserID", Type::Int64}, {"Name", Type::String}});
    auto path = MakeTempDb("test_scan_proj.skewdb", schema, {
        {"1", "a"},
        {"2", "b"},
    });

    Scan scan(path, Schema({{"UserID", Type::Int64}}));
    auto block = scan.Next();

    ASSERT_TRUE(block.has_value());
    ASSERT_EQ(block->row_count, 2);
    ASSERT_EQ(block->names.size(), 1);
    EXPECT_EQ(block->columns[0]->Get(0), Value{int64_t{1}});
    EXPECT_EQ(block->columns[0]->Get(1), Value{int64_t{2}});
    EXPECT_FALSE(scan.Next().has_value());
}
