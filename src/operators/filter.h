#pragma once

#include <memory>
#include "operators/expression.h"
#include "operators/operator.h"

class Filter : public Operator {
public:
    Filter(std::unique_ptr<Operator> child, std::unique_ptr<Expression> predicate);
    std::optional<Block> Next() override;

private:
    std::unique_ptr<Operator> child_;
    std::unique_ptr<Expression> predicate_;
};
