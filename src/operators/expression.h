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

using UnaryFn = std::function<Value(const Value&)>;
using BinaryFn = std::function<Value(const Value&, const Value&)>;
using TernaryFn = std::function<Value(const Value&, const Value&, const Value&)>;

class UnaryOp : public Expression {
public:
    UnaryOp(std::unique_ptr<Expression> arg, UnaryFn fn);
    Value Evaluate(const Block& block, size_t row_idx) const override;

private:
    std::unique_ptr<Expression> arg_;
    UnaryFn fn_;
};

class BinaryOp : public Expression {
public:
    BinaryOp(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right, BinaryFn fn);
    Value Evaluate(const Block& block, size_t row_idx) const override;

private:
    std::unique_ptr<Expression> left_;
    std::unique_ptr<Expression> right_;
    BinaryFn fn_;
};

class TernaryOp : public Expression {
public:
    TernaryOp(std::unique_ptr<Expression> a, std::unique_ptr<Expression> b,
              std::unique_ptr<Expression> c, TernaryFn fn);
    Value Evaluate(const Block& block, size_t row_idx) const override;

private:
    std::unique_ptr<Expression> a_, b_, c_;
    TernaryFn fn_;
};

std::unique_ptr<Expression> MakeRef(std::string name);
std::unique_ptr<Expression> MakeConst(Value value);

std::unique_ptr<Expression> Eq(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r);
std::unique_ptr<Expression> Ne(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r);
std::unique_ptr<Expression> Gt(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r);
std::unique_ptr<Expression> And(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r);
std::unique_ptr<Expression> Like(std::unique_ptr<Expression> arg, std::string pattern);
std::unique_ptr<Expression> NotLike(std::unique_ptr<Expression> arg, std::string pattern);
std::unique_ptr<Expression> StringLength(std::unique_ptr<Expression> arg);
std::unique_ptr<Expression> ExtractMinute(std::unique_ptr<Expression> arg);
std::unique_ptr<Expression> Le(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r);
std::unique_ptr<Expression> Ge(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r);
std::unique_ptr<Expression> Or(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r);
std::unique_ptr<Expression> Plus(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r);
std::unique_ptr<Expression> CaseWhen(std::unique_ptr<Expression> cond,
                                     std::unique_ptr<Expression> then,
                                     std::unique_ptr<Expression> otherwise);
std::unique_ptr<Expression> DateTruncMinute(std::unique_ptr<Expression> arg);
std::unique_ptr<Expression> ExtractDomain(std::unique_ptr<Expression> arg);
