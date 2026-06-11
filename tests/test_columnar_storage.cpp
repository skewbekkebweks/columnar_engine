#include <gtest/gtest.h>
#include <climits>
#include <vector>

#include "column.h"
#include "columnar_reader.h"
#include "columnar_writer.h"
#include "schema.h"
#include "utility.h"

TEST(Storage, RoundtripAllTypes) {
    Schema schema({
        {"i", Type::Int64},
        {"s", Type::String},
        {"ts", Type::Timestamp},
        {"d", Type::Date}
    });
    {
        ColumnarWriter writer("/tmp/test_rt_all.skewdb", schema);
        auto cols = MakeCols(schema, {{"100", "hello", "2026-05-28 12:00:00", "2026-05-28"}});
        writer.AddRowGroup(cols);
        std::move(writer).Finalize();
    }

    ColumnarReader reader("/tmp/test_rt_all.skewdb");

    const auto& s = reader.GetSchema();
    ASSERT_EQ(s.Size(), 4);
    EXPECT_EQ(s.GetColumns()[0].name, "i");
    EXPECT_EQ(s.GetColumns()[0].type, Type::Int64);
    EXPECT_EQ(s.GetColumns()[1].name, "s");
    EXPECT_EQ(s.GetColumns()[2].name, "ts");
    EXPECT_EQ(s.GetColumns()[3].name, "d");

    auto rg = reader.ReadRowGroup();
    ASSERT_TRUE(rg.has_value());
    ASSERT_EQ(rg->size(), 4);
    EXPECT_EQ((*rg)[0]->Get(0), Value{int64_t{100}});
    EXPECT_EQ((*rg)[1]->Get(0), Value{std::string{"hello"}});
    EXPECT_EQ((*rg)[2]->Get(0), Value{std::string{"2026-05-28 12:00:00"}});
    EXPECT_EQ((*rg)[3]->Get(0), Value{std::string{"2026-05-28"}});
}

TEST(Storage, MultipleRowGroups) {
    Schema schema({{"v", Type::Int64}});
    {
        ColumnarWriter writer("/tmp/test_rt_multi.skewdb", schema);

        auto col1 = std::make_unique<ColumnInt64>();
        col1->PushBack(Value{int64_t{1}});
        col1->PushBack(Value{int64_t{2}});
        std::vector<std::unique_ptr<Column>> rg1;
        rg1.push_back(std::move(col1));
        writer.AddRowGroup(rg1);

        auto col2 = std::make_unique<ColumnInt64>();
        col2->PushBack(Value{int64_t{3}});
        std::vector<std::unique_ptr<Column>> rg2;
        rg2.push_back(std::move(col2));
        writer.AddRowGroup(rg2);

        std::move(writer).Finalize();
    }

    ColumnarReader reader("/tmp/test_rt_multi.skewdb");
    EXPECT_EQ(reader.GetTotalRowCount(), 3);

    auto rg_a = reader.ReadRowGroup();
    ASSERT_TRUE(rg_a.has_value());
    EXPECT_EQ((*rg_a)[0]->Size(), 2);
    EXPECT_EQ((*rg_a)[0]->Get(0), Value{int64_t{1}});
    EXPECT_EQ((*rg_a)[0]->Get(1), Value{int64_t{2}});

    auto rg_b = reader.ReadRowGroup();
    ASSERT_TRUE(rg_b.has_value());
    EXPECT_EQ((*rg_b)[0]->Size(), 1);
    EXPECT_EQ((*rg_b)[0]->Get(0), Value{int64_t{3}});

    EXPECT_FALSE(reader.HasRowGroup());
}

TEST(Storage, ColumnProjection) {
    Schema schema({{"a", Type::Int64}, {"b", Type::Int64}, {"c", Type::Int64}});
    {
        ColumnarWriter writer("/tmp/test_rt_proj.skewdb", schema);
        auto cols = MakeCols(schema, {{"1", "2", "3"}, {"4", "5", "6"}});
        writer.AddRowGroup(cols);
        std::move(writer).Finalize();
    }

    ColumnarReader reader("/tmp/test_rt_proj.skewdb");
    std::vector<size_t> indices = {2};
    auto rg = reader.ReadRowGroup(&indices);
    ASSERT_TRUE(rg.has_value());
    ASSERT_EQ(rg->size(), 1);
    EXPECT_EQ((*rg)[0]->Get(0), Value{int64_t{3}});
    EXPECT_EQ((*rg)[0]->Get(1), Value{int64_t{6}});
}
