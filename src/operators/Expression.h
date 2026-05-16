#pragma once

#include <functional>
#include <memory>
#include <string>
#include "operators/block.h"
#include "value.h"

class Expression {
public:
    virtual ~Expression() = default;
    virtual Value Evaluate(const Block& block, size_t row_idx) const = 0;
};

class ColumnRef : public Expression {
public:
    explicit ColumnRef(std::string name);
    Value Evaluate(const Block& block, size_t row_idx) const override;

private:
    std::string name_;
};

class Constant : public Expression {
public:
    explicit Constant(Value value);
    Value Evaluate(const Block& block, size_t row_idx) const override;

private:
    Value value_;
};

using BinaryFn = std::function<Value(const Value&, const Value&)>;

class BinaryOp : public Expression {
public:
    BinaryOp(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right, BinaryFn fn);
    Value Evaluate(const Block& block, size_t row_idx) const override;

private:
    std::unique_ptr<Expression> left_;
    std::unique_ptr<Expression> right_;
    BinaryFn fn_;
};

std::unique_ptr<Expression> MakeRef(std::string name);
std::unique_ptr<Expression> MakeConst(Value value);

std::unique_ptr<Expression> Ne(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r);
