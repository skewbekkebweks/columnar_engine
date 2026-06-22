#include "operators/expression.h"

ColumnRef::ColumnRef(std::string name) : name_(std::move(name)) {
}

Value ColumnRef::Evaluate(const Block& block, size_t row_idx) const {
    if (!cached_idx_) {
        for (size_t i = 0; i < block.names.size(); ++i) {
            if (block.names[i] == name_) {
                cached_idx_ = i;
                break;
            }
        }
    }
    return block.columns[*cached_idx_]->Get(row_idx);
}

Constant::Constant(Value value) : value_(std::move(value)) {
}

Value Constant::Evaluate(const Block&, size_t) const {
    return value_;
}

BinaryOp::BinaryOp(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right, BinaryFn fn)
    : left_(std::move(left)), right_(std::move(right)), fn_(std::move(fn)) {
}

Value BinaryOp::Evaluate(const Block& block, size_t row_idx) const {
    return fn_(left_->Evaluate(block, row_idx), right_->Evaluate(block, row_idx));
}

std::unique_ptr<Expression> MakeRef(std::string name) {
    return std::make_unique<ColumnRef>(std::move(name));
}

std::unique_ptr<Expression> MakeConst(Value value) {
    return std::make_unique<Constant>(std::move(value));
}

static Value BoolVal(bool b) {
    return Value{int64_t{b ? 1 : 0}};
}

UnaryOp::UnaryOp(std::unique_ptr<Expression> arg, UnaryFn fn)
    : arg_(std::move(arg)), fn_(std::move(fn)) {
}

Value UnaryOp::Evaluate(const Block& block, size_t row_idx) const {
    return fn_(arg_->Evaluate(block, row_idx));
}

std::unique_ptr<Expression> Eq(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r) {
    return std::make_unique<BinaryOp>(
        std::move(l), std::move(r),
        [](const Value& a, const Value& b) { return BoolVal(Equal(a, b)); });
}

std::unique_ptr<Expression> Ne(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r) {
    return std::make_unique<BinaryOp>(
        std::move(l), std::move(r),
        [](const Value& a, const Value& b) { return BoolVal(NotEqual(a, b)); });
}

std::unique_ptr<Expression> Gt(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r) {
    return std::make_unique<BinaryOp>(
        std::move(l), std::move(r),
        [](const Value& a, const Value& b) { return BoolVal(Greater(a, b)); });
}

namespace {

class AndOp : public Expression {
public:
    AndOp(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r)
        : l_(std::move(l)), r_(std::move(r)) {
    }

    Value Evaluate(const Block& block, size_t row_idx) const override {
        if (IsZero(l_->Evaluate(block, row_idx))) {
            return int64_t{0};
        }
        return BoolVal(!IsZero(r_->Evaluate(block, row_idx)));
    }

private:
    std::unique_ptr<Expression> l_, r_;
};

class OrOp : public Expression {
public:
    OrOp(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r)
        : l_(std::move(l)), r_(std::move(r)) {
    }

    Value Evaluate(const Block& block, size_t row_idx) const override {
        if (!IsZero(l_->Evaluate(block, row_idx))) {
            return int64_t{1};
        }
        return BoolVal(!IsZero(r_->Evaluate(block, row_idx)));
    }

private:
    std::unique_ptr<Expression> l_, r_;
};

}

std::unique_ptr<Expression> And(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r) {
    return std::make_unique<AndOp>(std::move(l), std::move(r));
}

// работает только для "%text%"
static bool LikeMatch(const std::string& text, const std::string& pattern) {
    return text.find(pattern.data() + 1, 0, pattern.size() - 2) != std::string::npos;
}

std::unique_ptr<Expression> Like(std::unique_ptr<Expression> arg, std::string pattern) {
    return std::make_unique<UnaryOp>(std::move(arg),
                                     [pat = std::move(pattern)](const Value& v) -> Value {
                                         return BoolVal(LikeMatch(std::get<std::string>(v), pat));
                                     });
}

std::unique_ptr<Expression> NotLike(std::unique_ptr<Expression> arg, std::string pattern) {
    return std::make_unique<UnaryOp>(std::move(arg),
                                     [pat = std::move(pattern)](const Value& v) -> Value {
                                         return BoolVal(!LikeMatch(std::get<std::string>(v), pat));
                                     });
}

std::unique_ptr<Expression> StringLength(std::unique_ptr<Expression> arg) {
    return std::make_unique<UnaryOp>(std::move(arg), [](const Value& v) -> Value {
        return int64_t{static_cast<int64_t>(std::get<std::string>(v).size())};
    });
}

std::unique_ptr<Expression> ExtractMinute(std::unique_ptr<Expression> arg) {
    return std::make_unique<UnaryOp>(std::move(arg), [](const Value& v) -> Value {
        const std::string& s = std::get<std::string>(v);
        return int64_t{std::stoll(s.substr(14, 2))};
    });
}

TernaryOp::TernaryOp(std::unique_ptr<Expression> a, std::unique_ptr<Expression> b,
                     std::unique_ptr<Expression> c, TernaryFn fn)
    : a_(std::move(a)), b_(std::move(b)), c_(std::move(c)), fn_(std::move(fn)) {
}

Value TernaryOp::Evaluate(const Block& block, size_t row_idx) const {
    return fn_(a_->Evaluate(block, row_idx), b_->Evaluate(block, row_idx),
               c_->Evaluate(block, row_idx));
}

std::unique_ptr<Expression> Le(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r) {
    return std::make_unique<BinaryOp>(
        std::move(l), std::move(r),
        [](const Value& a, const Value& b) { return BoolVal(LessOrEqual(a, b)); });
}

std::unique_ptr<Expression> Ge(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r) {
    return std::make_unique<BinaryOp>(
        std::move(l), std::move(r),
        [](const Value& a, const Value& b) { return BoolVal(GreaterOrEqual(a, b)); });
}

std::unique_ptr<Expression> Or(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r) {
    return std::make_unique<OrOp>(std::move(l), std::move(r));
}

std::unique_ptr<Expression> Plus(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r) {
    return std::make_unique<BinaryOp>(std::move(l), std::move(r),
                                      [](const Value& a, const Value& b) { return Add(a, b); });
}

std::unique_ptr<Expression> CaseWhen(std::unique_ptr<Expression> cond,
                                     std::unique_ptr<Expression> then,
                                     std::unique_ptr<Expression> otherwise) {
    return std::make_unique<TernaryOp>(
        std::move(cond), std::move(then), std::move(otherwise),
        [](const Value& c, const Value& t, const Value& e) { return !IsZero(c) ? t : e; });
}

std::unique_ptr<Expression> DateTruncMinute(std::unique_ptr<Expression> arg) {
    return std::make_unique<UnaryOp>(std::move(arg), [](const Value& v) -> Value {
        const std::string& s = std::get<std::string>(v);
        return s.substr(0, 16) + ":00";
    });
}

std::unique_ptr<Expression> ExtractDomain(std::unique_ptr<Expression> arg) {
    return std::make_unique<UnaryOp>(std::move(arg), [](const Value& v) -> Value {
        const std::string& url = std::get<std::string>(v);
        size_t nl = url.find('\n');
        if (nl != std::string::npos && nl != url.size() - 1) {
            return url;
        }
        size_t start = url.find("://");
        if (start == std::string::npos) {
            return url;
        }
        start += 3;
        if (url.compare(start, 4, "www.") == 0) {
            start += 4;
        }
        size_t end = url.find('/', start);
        if (end == std::string::npos) {
            return url;
        }
        return url.substr(start, end - start);
    });
}
