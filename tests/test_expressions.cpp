#include <gtest/gtest.h>

#include "operators/block.h"
#include "operators/expression.h"
#include "column.h"
#include "utility.h"

TEST(Expressions, ColumnRef) {
    auto col = std::make_unique<ColumnInt64>();
    col->PushBack(Value{int64_t{10}});
    col->PushBack(Value{int64_t{20}});
    Block b;
    b.row_count = 2;
    b.AddColumn("x", std::move(col));

    auto expr = MakeRef("x");
    EXPECT_EQ(expr->Evaluate(b, 0), Value{int64_t{10}});
    EXPECT_EQ(expr->Evaluate(b, 1), Value{int64_t{20}});
}

TEST(Expressions, Constant) {
    Block b;
    b.row_count = 0;
    EXPECT_EQ(MakeConst(Value{int64_t{42}})->Evaluate(b, 0), Value{int64_t{42}});
    EXPECT_EQ(MakeConst(Value{std::string{"hi"}})->Evaluate(b, 0), Value{std::string{"hi"}});
}

TEST(Expressions, Eq) {
    auto b = OneRow({{"a", 5}, {"b", 5}, {"c", 3}});
    EXPECT_EQ(Eq(MakeRef("a"), MakeRef("b"))->Evaluate(b, 0), Value{int64_t{1}});
    EXPECT_EQ(Eq(MakeRef("a"), MakeRef("c"))->Evaluate(b, 0), Value{int64_t{0}});
}

TEST(Expressions, Ne) {
    auto b = OneRow({{"a", 5}, {"b", 5}, {"c", 3}});
    EXPECT_EQ(Ne(MakeRef("a"), MakeRef("b"))->Evaluate(b, 0), Value{int64_t{0}});
    EXPECT_EQ(Ne(MakeRef("a"), MakeRef("c"))->Evaluate(b, 0), Value{int64_t{1}});
}

TEST(Expressions, Gt) {
    auto b = OneRow({{"a", 5}, {"b", 3}, {"c", 7}});
    EXPECT_EQ(Gt(MakeRef("a"), MakeRef("b"))->Evaluate(b, 0), Value{int64_t{1}});
    EXPECT_EQ(Gt(MakeRef("a"), MakeRef("c"))->Evaluate(b, 0), Value{int64_t{0}});
    EXPECT_EQ(Gt(MakeRef("a"), MakeRef("a"))->Evaluate(b, 0), Value{int64_t{0}});
}

TEST(Expressions, Le) {
    auto b = OneRow({{"a", 5}, {"b", 5}, {"c", 3}});
    EXPECT_EQ(Le(MakeRef("a"), MakeRef("b"))->Evaluate(b, 0), Value{int64_t{1}});
    EXPECT_EQ(Le(MakeRef("c"), MakeRef("a"))->Evaluate(b, 0), Value{int64_t{1}});
    EXPECT_EQ(Le(MakeRef("a"), MakeRef("c"))->Evaluate(b, 0), Value{int64_t{0}});
}

TEST(Expressions, Ge) {
    auto b = OneRow({{"a", 5}, {"b", 5}, {"c", 3}});
    EXPECT_EQ(Ge(MakeRef("a"), MakeRef("b"))->Evaluate(b, 0), Value{int64_t{1}});
    EXPECT_EQ(Ge(MakeRef("a"), MakeRef("c"))->Evaluate(b, 0), Value{int64_t{1}});
    EXPECT_EQ(Ge(MakeRef("c"), MakeRef("a"))->Evaluate(b, 0), Value{int64_t{0}});
}

TEST(Expressions, And) {
    auto b = OneRow({{"t", 1}, {"f", 0}});
    EXPECT_EQ(And(MakeRef("t"), MakeRef("t"))->Evaluate(b, 0), Value{int64_t{1}});
    EXPECT_EQ(And(MakeRef("t"), MakeRef("f"))->Evaluate(b, 0), Value{int64_t{0}});
    EXPECT_EQ(And(MakeRef("f"), MakeRef("f"))->Evaluate(b, 0), Value{int64_t{0}});
}

TEST(Expressions, Or) {
    auto b = OneRow({{"t", 1}, {"f", 0}});
    EXPECT_EQ(Or(MakeRef("f"), MakeRef("f"))->Evaluate(b, 0), Value{int64_t{0}});
    EXPECT_EQ(Or(MakeRef("t"), MakeRef("f"))->Evaluate(b, 0), Value{int64_t{1}});
    EXPECT_EQ(Or(MakeRef("t"), MakeRef("t"))->Evaluate(b, 0), Value{int64_t{1}});
}

TEST(Expressions, Plus) {
    auto b = OneRow({{"a", 3}, {"b", 7}});
    EXPECT_EQ(Plus(MakeRef("a"), MakeRef("b"))->Evaluate(b, 0), Value{__int128{10}});
    EXPECT_EQ(Plus(MakeRef("a"), MakeConst(Value{int64_t{-3}}))->Evaluate(b, 0),
              Value{__int128{0}});
}

TEST(Expressions, Like) {
    auto b = OneRow({}, {{"s", "hello world"}});
    EXPECT_EQ(Like(MakeRef("s"), "%world%")->Evaluate(b, 0), Value{int64_t{1}});
    EXPECT_EQ(Like(MakeRef("s"), "%xyz%")->Evaluate(b, 0), Value{int64_t{0}});
}

TEST(Expressions, NotLike) {
    auto b = OneRow({}, {{"s", "hello world"}});
    EXPECT_EQ(NotLike(MakeRef("s"), "%world%")->Evaluate(b, 0), Value{int64_t{0}});
    EXPECT_EQ(NotLike(MakeRef("s"), "%xyz%")->Evaluate(b, 0), Value{int64_t{1}});
}

TEST(Expressions, StringLength) {
    auto b = OneRow({}, {{"s", "hello"}});
    EXPECT_EQ(StringLength(MakeRef("s"))->Evaluate(b, 0), Value{int64_t{5}});

    auto empty = OneRow({}, {{"s", ""}});
    EXPECT_EQ(StringLength(MakeRef("s"))->Evaluate(empty, 0), Value{int64_t{0}});
}

TEST(Expressions, ExtractMinute) {
    auto b = TimestampBlock("2013-07-15 12:34:00");
    EXPECT_EQ(ExtractMinute(MakeRef("t"))->Evaluate(b, 0), Value{int64_t{34}});

    auto b2 = TimestampBlock("2013-07-15 00:07:00");
    EXPECT_EQ(ExtractMinute(MakeRef("t"))->Evaluate(b2, 0), Value{int64_t{7}});
}

TEST(Expressions, CaseWhen) {
    auto b1 = OneRow({{"cond", 1}, {"a", 10}, {"b", 20}});
    EXPECT_EQ(CaseWhen(MakeRef("cond"), MakeRef("a"), MakeRef("b"))->Evaluate(b1, 0),
              Value{int64_t{10}});

    auto b2 = OneRow({{"cond", 0}, {"a", 10}, {"b", 20}});
    EXPECT_EQ(CaseWhen(MakeRef("cond"), MakeRef("a"), MakeRef("b"))->Evaluate(b2, 0),
              Value{int64_t{20}});
}

TEST(Expressions, DateTruncMinute) {
    auto b = TimestampBlock("2013-07-15 12:34:56");
    auto v = DateTruncMinute(MakeRef("t"))->Evaluate(b, 0);
    EXPECT_EQ(FormatTimestamp(std::get<int64_t>(v)), "2013-07-15 12:34:00");
}

TEST(Expressions, ExtractDomain) {
    auto eval = [](const std::string& url) -> std::string {
        auto b = OneRow({}, {{"url", url}});
        return std::get<std::string>(ExtractDomain(MakeRef("url"))->Evaluate(b, 0));
    };

    EXPECT_EQ(eval("http://example.com/path"), "example.com");
    EXPECT_EQ(eval("http://www.example.com/path"), "example.com");
    EXPECT_EQ(eval("https://sub.domain.com/a/b/c"), "sub.domain.com");
    EXPECT_EQ(eval("http://example.com"), "http://example.com");
    EXPECT_EQ(eval("not-a-url"), "not-a-url");
    EXPECT_EQ(eval("http://ex\nample.com/path"), "http://ex\nample.com/path");
    EXPECT_EQ(eval("http://example.com/path\n"), "example.com");
}
