#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <variant>

#include "type.h"

using Value = std::variant<int64_t, __int128, std::string>;

Type TypeOf(const Value& v);

Value Add(const Value& a, const Value& b);
Value Div(const Value& a, const Value& b);

bool Equal(const Value& a, const Value& b);
bool NotEqual(const Value& a, const Value& b);
bool Less(const Value& a, const Value& b);
bool Greater(const Value& a, const Value& b);
bool LessOrEqual(const Value& a, const Value& b);
bool GreaterOrEqual(const Value& a, const Value& b);

bool IsZero(const Value& v);

std::string ToString(const Value& v);

struct ValueHash {
    size_t operator()(const Value& v) const;
};
