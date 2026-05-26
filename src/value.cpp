#include "value.h"

#include "error.h"

template <typename T>
static constexpr bool kIsNumeric = std::is_arithmetic_v<T> || std::is_same_v<T, __int128>;

static std::string Int128ToString(__int128 x) {
    if (x == 0) {
        return "0";
    }
    bool negative = x < 0;
    if (negative) {
        x = -x;
    }
    std::string result;
    while (x > 0) {
        result = char('0' + x % 10) + result;
        x /= 10;
    }
    if (negative) {
        result = '-' + result;
    }
    return result;
}

Value Add(const Value& a, const Value& b) {
    return std::visit(
        [](auto&& x, auto&& y) -> Value {
            using X = std::decay_t<decltype(x)>;
            using Y = std::decay_t<decltype(y)>;
            if constexpr (kIsNumeric<X> && kIsNumeric<Y>) {
                return static_cast<__int128>(x) + static_cast<__int128>(y);
            }
            THROW_NOT_IMPLEMENTED;
        },
        a, b);
}

Type TypeOf(const Value& v) {
    if (std::holds_alternative<int64_t>(v)) {
        return Type::Int64;
    }
    if (std::holds_alternative<__int128>(v)) {
        return Type::Int128;
    }
    return Type::String;
}

Value Div(const Value& a, const Value& b) {
    return std::visit(
        [](auto&& x, auto&& y) -> Value {
            using X = std::decay_t<decltype(x)>;
            using Y = std::decay_t<decltype(y)>;
            if constexpr (kIsNumeric<X> && kIsNumeric<Y>) {
                if (y == Y{0}) {
                    THROW_RUNTIME_ERROR("Division by zero");
                }
                return static_cast<__int128>(x) / static_cast<__int128>(y);
            }
            THROW_NOT_IMPLEMENTED;
        },
        a, b);
}

bool Equal(const Value& a, const Value& b) {
    return a == b;
}

bool NotEqual(const Value& a, const Value& b) {
    return a != b;
}

bool Less(const Value& a, const Value& b) {
    return std::visit(
        [](auto&& x, auto&& y) -> bool {
            using X = std::decay_t<decltype(x)>;
            using Y = std::decay_t<decltype(y)>;
            if constexpr (std::is_same_v<X, Y>) {
                return x < y;
            }
            THROW_NOT_IMPLEMENTED;
        },
        a, b);
}

bool Greater(const Value& a, const Value& b) {
    return Less(b, a);
}

bool LessOrEqual(const Value& a, const Value& b) {
    return !Less(b, a);
}

bool GreaterOrEqual(const Value& a, const Value& b) {
    return !Less(a, b);
}

bool IsZero(const Value& v) {
    return std::visit(
        [](auto&& x) -> bool {
            using X = std::decay_t<decltype(x)>;
            if constexpr (kIsNumeric<X>) {
                return x == X{0};
            }
            return false;
        },
        v);
}

std::string ToString(const Value& v) {
    return std::visit(
        [](auto&& x) -> std::string {
            using X = std::decay_t<decltype(x)>;
            if constexpr (std::is_same_v<X, std::string>) {
                return x;
            } else if constexpr (std::is_same_v<X, __int128>) {
                return Int128ToString(x);
            } else {
                return std::to_string(x);
            }
        },
        v);
}

size_t ValueHash::operator()(const Value& v) const {
    return std::visit(
        [](auto&& x) -> size_t {
            using X = std::decay_t<decltype(x)>;
            if constexpr (std::is_same_v<X, __int128>) {
                return std::hash<uint64_t>{}(static_cast<uint64_t>(x)) ^
                       std::hash<uint64_t>{}(
                           static_cast<uint64_t>(static_cast<unsigned __int128>(x) >> 64));
            } else {
                return std::hash<X>{}(x);
            }
        },
        v);
}
