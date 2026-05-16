#pragma once

#include <memory>
#include <string>
#include "operators/Operator.h"

class Count : public Operator {
public:
    Count(std::unique_ptr<Operator> child, std::string alias = "count");
    std::optional<Block> Next() override;

private:
    std::unique_ptr<Operator> child_;
    std::string alias_;
    bool done_ = false;
};
