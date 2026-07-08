#include <gtest/gtest.h>
#include <algorithm>
#include <vector>

#include "operators/aggregate.h"
#include "operators/aggregator.h"
#include "operators/count.h"
#include "operators/expression.h"
#include "operators/filter.h"
#include "operators/group_by.h"
#include "operators/limit.h"
#include "operators/order_by.h"
#include "operators/project.h"
#include "operators/scan.h"
#include "value.h"
#include "utility.h"

TEST(Operators, Q0) {
    Schema schema({{"UserID", Type::Int64}, {"Name", Type::String}});
    auto path = MakeTempDb("test_q1.skewdb", schema,
                           {
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
    auto path = MakeTempDb("test_scan_proj.skewdb", schema,
                           {
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

TEST(Operators, Filter) {
    Schema schema({{"id", Type::Int64}, {"val", Type::Int64}});
    auto path = MakeTempDb("test_filter.skewdb", schema, {{"1", "10"}, {"2", "20"}, {"3", "30"}});

    Filter f(std::make_unique<Scan>(path), Ge(MakeRef("val"), MakeConst(Value{int64_t{20}})));

    auto block = f.Next();
    ASSERT_TRUE(block.has_value());
    EXPECT_EQ(block->row_count, 2);
    EXPECT_EQ(block->columns[1]->Get(0), Value{int64_t{20}});
    EXPECT_EQ(block->columns[1]->Get(1), Value{int64_t{30}});
    EXPECT_FALSE(f.Next().has_value());
}

TEST(Operators, FilterNonePass) {
    Schema schema({{"val", Type::Int64}});
    auto path = MakeTempDb("test_filter_none.skewdb", schema, {{"1"}, {"2"}});

    Filter f(std::make_unique<Scan>(path), Gt(MakeRef("val"), MakeConst(Value{int64_t{100}})));

    EXPECT_FALSE(f.Next().has_value());
}

TEST(Operators, Limit) {
    Schema schema({{"val", Type::Int64}});
    auto path = MakeTempDb("test_limit.skewdb", schema, {{"10"}, {"20"}, {"30"}, {"40"}, {"50"}});

    Limit lim(std::make_unique<Scan>(path), 3);
    auto block = lim.Next();

    ASSERT_TRUE(block.has_value());
    EXPECT_EQ(block->row_count, 3);
    EXPECT_EQ(block->columns[0]->Get(0), Value{int64_t{10}});
    EXPECT_EQ(block->columns[0]->Get(2), Value{int64_t{30}});
    EXPECT_FALSE(lim.Next().has_value());
}

TEST(Operators, LimitExceedsRows) {
    Schema schema({{"val", Type::Int64}});
    auto path = MakeTempDb("test_limit_exceed.skewdb", schema, {{"10"}, {"20"}});

    Limit lim(std::make_unique<Scan>(path), 100);
    auto block = lim.Next();

    ASSERT_TRUE(block.has_value());
    EXPECT_EQ(block->row_count, 2);
}

TEST(Operators, LimitWithOffset) {
    Schema schema({{"val", Type::Int64}});
    auto path =
        MakeTempDb("test_limit_offset.skewdb", schema, {{"10"}, {"20"}, {"30"}, {"40"}, {"50"}});

    Limit lim(std::make_unique<Scan>(path), 2, 2);
    auto block = lim.Next();

    ASSERT_TRUE(block.has_value());
    EXPECT_EQ(block->row_count, 2);
    EXPECT_EQ(block->columns[0]->Get(0), Value{int64_t{30}});
    EXPECT_EQ(block->columns[0]->Get(1), Value{int64_t{40}});
}

TEST(Operators, Project) {
    Schema schema({{"a", Type::Int64}, {"b", Type::Int64}, {"c", Type::Int64}});
    auto path = MakeTempDb("test_project.skewdb", schema, {{"1", "2", "3"}});

    Project proj(std::make_unique<Scan>(path), {"c", "a"});
    auto block = proj.Next();

    ASSERT_TRUE(block.has_value());
    ASSERT_EQ(block->names.size(), 2);
    EXPECT_EQ(block->names[0], "c");
    EXPECT_EQ(block->names[1], "a");
    EXPECT_EQ(block->columns[0]->Get(0), Value{int64_t{3}});
    EXPECT_EQ(block->columns[1]->Get(0), Value{int64_t{1}});
}

TEST(Operators, OrderByAsc) {
    Schema schema({{"val", Type::Int64}});
    auto path = MakeTempDb("test_orderby_asc.skewdb", schema, {{"30"}, {"10"}, {"20"}});

    OrderBy ob(std::make_unique<Scan>(path), {{"val", false}});
    auto block = ob.Next();

    ASSERT_TRUE(block.has_value());
    EXPECT_EQ(block->row_count, 3);
    EXPECT_EQ(block->columns[0]->Get(0), Value{int64_t{10}});
    EXPECT_EQ(block->columns[0]->Get(1), Value{int64_t{20}});
    EXPECT_EQ(block->columns[0]->Get(2), Value{int64_t{30}});
}

TEST(Operators, OrderByDesc) {
    Schema schema({{"val", Type::Int64}});
    auto path = MakeTempDb("test_orderby_desc.skewdb", schema, {{"30"}, {"10"}, {"20"}});

    OrderBy ob(std::make_unique<Scan>(path), {{"val", true}});
    auto block = ob.Next();

    ASSERT_TRUE(block.has_value());
    EXPECT_EQ(block->row_count, 3);
    EXPECT_EQ(block->columns[0]->Get(0), Value{int64_t{30}});
    EXPECT_EQ(block->columns[0]->Get(1), Value{int64_t{20}});
    EXPECT_EQ(block->columns[0]->Get(2), Value{int64_t{10}});
}

TEST(Operators, OrderByWithLimit) {
    Schema schema({{"val", Type::Int64}});
    auto path =
        MakeTempDb("test_orderby_limit.skewdb", schema, {{"30"}, {"10"}, {"20"}, {"40"}, {"50"}});

    OrderBy ob(std::make_unique<Scan>(path), {{"val", false}}, 3);
    auto block = ob.Next();

    ASSERT_TRUE(block.has_value());
    EXPECT_EQ(block->row_count, 3);
    EXPECT_EQ(block->columns[0]->Get(0), Value{int64_t{10}});
    EXPECT_EQ(block->columns[0]->Get(1), Value{int64_t{20}});
    EXPECT_EQ(block->columns[0]->Get(2), Value{int64_t{30}});
}

TEST(Operators, OrderByWithOffset) {
    Schema schema({{"val", Type::Int64}});
    auto path =
        MakeTempDb("test_orderby_offset.skewdb", schema, {{"30"}, {"10"}, {"20"}, {"40"}, {"50"}});

    OrderBy ob(std::make_unique<Scan>(path), {{"val", false}}, 3, 1);
    auto block = ob.Next();

    ASSERT_TRUE(block.has_value());
    EXPECT_EQ(block->row_count, 3);
    EXPECT_EQ(block->columns[0]->Get(0), Value{int64_t{20}});
    EXPECT_EQ(block->columns[0]->Get(1), Value{int64_t{30}});
    EXPECT_EQ(block->columns[0]->Get(2), Value{int64_t{40}});
}

TEST(Operators, AggregateSumMinMax) {
    Schema schema({{"val", Type::Int64}});
    auto path = MakeTempDb("test_agg_smm.skewdb", schema, {{"10"}, {"20"}, {"30"}});

    std::vector<std::unique_ptr<Aggregator>> aggs;
    aggs.push_back(MakeSumAgg(MakeRef("val")));
    aggs.push_back(MakeMinAgg(MakeRef("val")));
    aggs.push_back(MakeMaxAgg(MakeRef("val")));

    Aggregate agg(std::make_unique<Scan>(path), {"sum", "min", "max"}, std::move(aggs));
    auto block = agg.Next();

    ASSERT_TRUE(block.has_value());
    EXPECT_EQ(block->row_count, 1);
    EXPECT_EQ(block->columns[0]->Get(0), Value{__int128{60}});
    EXPECT_EQ(block->columns[1]->Get(0), Value{int64_t{10}});
    EXPECT_EQ(block->columns[2]->Get(0), Value{int64_t{30}});
    EXPECT_FALSE(agg.Next().has_value());
}

TEST(Operators, AggregateCountDistinct) {
    Schema schema({{"val", Type::Int64}});
    auto path = MakeTempDb("test_agg_distinct.skewdb", schema, {{"1"}, {"2"}, {"1"}, {"3"}, {"2"}});

    std::vector<std::unique_ptr<Aggregator>> aggs;
    aggs.push_back(MakeCountDistinctAgg(MakeRef("val")));

    Aggregate agg(std::make_unique<Scan>(path), {"cnt"}, std::move(aggs));
    auto block = agg.Next();

    ASSERT_TRUE(block.has_value());
    EXPECT_EQ(block->columns[0]->Get(0), Value{int64_t{3}});
}

TEST(Operators, GroupByCount) {
    Schema schema({{"grp", Type::Int64}, {"val", Type::Int64}});
    auto path = MakeTempDb("test_groupby_count.skewdb", schema,
                           {{"1", "10"}, {"2", "20"}, {"1", "30"}, {"2", "40"}});

    std::vector<std::unique_ptr<Expression>> keys;
    keys.push_back(MakeRef("grp"));
    GroupBy gb(std::make_unique<Scan>(path), std::move(keys), {"grp"}, {"cnt"},
               {[]() { return MakeCountAgg(); }});

    auto block = gb.Next();
    ASSERT_TRUE(block.has_value());
    EXPECT_EQ(block->row_count, 2);

    auto rows = Rows(*block);
    SortRows(rows);
    EXPECT_EQ(rows[0][0], Value{int64_t{1}});
    EXPECT_EQ(rows[0][1], Value{int64_t{2}});
    EXPECT_EQ(rows[1][0], Value{int64_t{2}});
    EXPECT_EQ(rows[1][1], Value{int64_t{2}});
    EXPECT_FALSE(gb.Next().has_value());
}

TEST(Operators, GroupBySum) {
    Schema schema({{"grp", Type::Int64}, {"val", Type::Int64}});
    auto path =
        MakeTempDb("test_groupby_sum.skewdb", schema, {{"1", "10"}, {"2", "20"}, {"1", "30"}});

    std::vector<std::unique_ptr<Expression>> keys;
    keys.push_back(MakeRef("grp"));
    GroupBy gb(std::make_unique<Scan>(path), std::move(keys), {"grp"}, {"sum"},
               {[]() { return MakeSumAgg(MakeRef("val")); }});

    auto block = gb.Next();
    ASSERT_TRUE(block.has_value());
    EXPECT_EQ(block->row_count, 2);

    auto rows = Rows(*block);
    SortRows(rows);
    EXPECT_EQ(rows[0][0], Value{int64_t{1}});
    EXPECT_EQ(rows[0][1], Value{__int128{40}});
    EXPECT_EQ(rows[1][0], Value{int64_t{2}});
    EXPECT_EQ(rows[1][1], Value{__int128{20}});
}

TEST(Operators, GroupByEmpty) {
    Schema schema({{"grp", Type::Int64}});
    auto path = MakeTempDb("test_groupby_empty.skewdb", schema, {});

    std::vector<std::unique_ptr<Expression>> keys;
    keys.push_back(MakeRef("grp"));
    GroupBy gb(std::make_unique<Scan>(path), std::move(keys), {"grp"}, {"cnt"},
               {[]() { return MakeCountAgg(); }});

    EXPECT_FALSE(gb.Next().has_value());
}
