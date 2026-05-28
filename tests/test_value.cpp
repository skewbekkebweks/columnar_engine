#include <gtest/gtest.h>

#include "type.h"
#include "value.h"

TEST(Value, TypeOf) {
    EXPECT_EQ(TypeOf(Value{int64_t{0}}), Type::Int64);
    EXPECT_EQ(TypeOf(Value{__int128{0}}), Type::Int128);
    EXPECT_EQ(TypeOf(Value{std::string{""}}), Type::String);
}

TEST(Value, Add) {
    EXPECT_EQ(Add(Value{int64_t{3}}, Value{int64_t{4}}), Value{__int128{7}});
    EXPECT_EQ(Add(Value{int64_t{-5}}, Value{int64_t{3}}), Value{__int128{-2}});
    EXPECT_EQ(Add(Value{__int128{100}}, Value{int64_t{50}}), Value{__int128{150}});

    int64_t big = INT64_MAX;
    EXPECT_EQ(Add(Value{big}, Value{int64_t{1}}), Value{__int128{((__int128)INT64_MAX) + 1}});
}

TEST(Value, Div) {
    EXPECT_EQ(Div(Value{int64_t{10}}, Value{int64_t{3}}), Value{__int128{3}});
    EXPECT_EQ(Div(Value{int64_t{-6}}, Value{int64_t{2}}), Value{__int128{-3}});
    EXPECT_ANY_THROW(Div(Value{int64_t{5}}, Value{int64_t{0}}));
}

TEST(Value, Equal) {
    EXPECT_TRUE(Equal(Value{int64_t{42}}, Value{int64_t{42}}));
    EXPECT_FALSE(Equal(Value{int64_t{1}}, Value{int64_t{2}}));
    EXPECT_TRUE(Equal(Value{std::string{"abc"}}, Value{std::string{"abc"}}));
    EXPECT_FALSE(Equal(Value{int64_t{0}}, Value{__int128{0}}));
}

TEST(Value, NotEqual) {
    EXPECT_TRUE(NotEqual(Value{int64_t{1}}, Value{int64_t{2}}));
    EXPECT_FALSE(NotEqual(Value{int64_t{1}}, Value{int64_t{1}}));
}

TEST(Value, Comparisons) {
    EXPECT_TRUE(Less(Value{int64_t{1}}, Value{int64_t{2}}));
    EXPECT_FALSE(Less(Value{int64_t{2}}, Value{int64_t{1}}));
    EXPECT_FALSE(Less(Value{int64_t{1}}, Value{int64_t{1}}));

    EXPECT_TRUE(Greater(Value{int64_t{2}}, Value{int64_t{1}}));
    EXPECT_FALSE(Greater(Value{int64_t{1}}, Value{int64_t{1}}));

    EXPECT_TRUE(LessOrEqual(Value{int64_t{1}}, Value{int64_t{1}}));
    EXPECT_TRUE(LessOrEqual(Value{int64_t{1}}, Value{int64_t{2}}));
    EXPECT_FALSE(LessOrEqual(Value{int64_t{2}}, Value{int64_t{1}}));

    EXPECT_TRUE(GreaterOrEqual(Value{int64_t{1}}, Value{int64_t{1}}));
    EXPECT_TRUE(GreaterOrEqual(Value{int64_t{2}}, Value{int64_t{1}}));

    EXPECT_TRUE(Less(Value{std::string{"abc"}}, Value{std::string{"abd"}}));
    EXPECT_FALSE(Less(Value{std::string{"b"}}, Value{std::string{"a"}}));
}

TEST(Value, ComparisonsMixedTypesThrow) {
    EXPECT_ANY_THROW(Less(Value{int64_t{1}}, Value{std::string{"a"}}));
    EXPECT_ANY_THROW(Less(Value{int64_t{0}}, Value{__int128{0}}));
}

TEST(Value, IsZero) {
    EXPECT_TRUE(IsZero(Value{int64_t{0}}));
    EXPECT_FALSE(IsZero(Value{int64_t{1}}));
    EXPECT_FALSE(IsZero(Value{int64_t{-1}}));
    EXPECT_TRUE(IsZero(Value{__int128{0}}));
    EXPECT_FALSE(IsZero(Value{__int128{1}}));
    EXPECT_FALSE(IsZero(Value{std::string{"0"}}));
    EXPECT_FALSE(IsZero(Value{std::string{""}}));
}

TEST(Value, ToString) {
    EXPECT_EQ(ToString(Value{int64_t{42}}), "42");
    EXPECT_EQ(ToString(Value{int64_t{-1}}), "-1");
    EXPECT_EQ(ToString(Value{int64_t{0}}), "0");
    EXPECT_EQ(ToString(Value{__int128{0}}), "0");
    EXPECT_EQ(ToString(Value{__int128{123456789}}), "123456789");
    EXPECT_EQ(ToString(Value{__int128{-42}}), "-42");
    EXPECT_EQ(ToString(Value{std::string{"hello"}}), "hello");
    EXPECT_EQ(ToString(Value{std::string{""}}), "");
}

TEST(Value, Hash) {
    ValueHash h;
    EXPECT_EQ(h(Value{int64_t{42}}), h(Value{int64_t{42}}));
    EXPECT_EQ(h(Value{std::string{"abc"}}), h(Value{std::string{"abc"}}));
    EXPECT_EQ(h(Value{__int128{99}}), h(Value{__int128{99}}));
}
