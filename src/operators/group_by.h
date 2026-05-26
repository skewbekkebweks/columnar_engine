#pragma once

#include <memory>
#include <string>
#include <vector>
#include "operators/aggregator.h"
#include "operators/block.h"
#include "operators/expression.h"
#include "operators/operator.h"

class GroupBy : public Operator {
public:
    GroupBy(std::unique_ptr<Operator> child, std::vector<std::unique_ptr<Expression>> key_exprs,
            std::vector<std::string> key_names, std::vector<std::string> agg_names,
            std::vector<AggFactory> agg_factories);
    std::optional<Block> Next() override;

private:
    std::unique_ptr<Operator> child_;
    std::vector<std::unique_ptr<Expression>> key_exprs_;
    std::vector<std::string> key_names_;
    std::vector<std::string> agg_names_;
    std::vector<AggFactory> agg_factories_;
    bool done_ = false;
};
