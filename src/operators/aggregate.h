#pragma once

#include <memory>
#include <string>
#include <vector>
#include "operators/aggregator.h"
#include "operators/operator.h"

class Aggregate : public Operator {
public:
    Aggregate(std::unique_ptr<Operator> child, std::vector<std::string> names,
              std::vector<std::unique_ptr<Aggregator>> aggs);
    std::optional<Block> Next() override;

private:
    std::unique_ptr<Operator> child_;
    std::vector<std::string> names_;
    std::vector<std::unique_ptr<Aggregator>> aggs_;
    bool done_ = false;
};
