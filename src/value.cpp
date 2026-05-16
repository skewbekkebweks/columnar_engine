#include "value.h"

#include "error.h"

Value Add(const Value& a, const Value& b) {
    return std::visit(
        [](auto&& x, auto&& y) -> Value {
            using X = std::decay_t<decltype(x)>;
            using Y = std::decay_t<decltype(y)>;
            if constexpr (std::is_same_v<X, Y> && std::is_arithmetic_v<X>) {
                return x + y;
            }
            THROW_NOT_IMPLEMENTED;
        },
        a, b);
}

Value Div(const Value& a, const Value& b) {
    return std::visit(
        [](auto&& x, auto&& y) -> Value {
            using X = std::decay_t<decltype(x)>;
            using Y = std::decay_t<decltype(y)>;
            if constexpr (std::is_arithmetic_v<X> && std::is_arithmetic_v<Y>) {
                if (y == Y{0}) {
                    THROW_RUNTIME_ERROR("Division by zero");
                }
                return static_cast<int64_t>(x) / static_cast<int64_t>(y);
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
            if constexpr (std::is_arithmetic_v<X>) {
                return x == X{0};
            }
            return false;
        },
        v);
}

bool IsEmpty(const Value& v) {
    if (const auto* s = std::get_if<std::string>(&v)) {
        return s->empty();
    }
    return false;
}

std::string ToString(const Value& v) {
    return std::visit(
        [](auto&& x) -> std::string {
            using X = std::decay_t<decltype(x)>;
            if constexpr (std::is_same_v<X, std::string>) {
                return x;
            } else {
                return std::to_string(x);
            }
        },
        v);
}

size_t ValueHash::operator()(const Value& v) const {
    return std::visit([](auto&& x) -> size_t { return std::hash<std::decay_t<decltype(x)>>{}(x); },
                      v);
}
