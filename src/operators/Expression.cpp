#include "operators/expression.h"

ColumnRef::ColumnRef(std::string name) : name_(std::move(name)) {
}

Value ColumnRef::Evaluate(const Block& block, size_t row_idx) const {
    return block.GetColumn(name_).Get(row_idx);
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

std::unique_ptr<Expression> Ne(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r) {
    return std::make_unique<BinaryOp>(
        std::move(l), std::move(r),
        [](const Value& a, const Value& b) { return BoolVal(NotEqual(a, b)); });
}
