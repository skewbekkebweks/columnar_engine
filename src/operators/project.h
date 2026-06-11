#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "operators/block.h"
#include "operators/operator.h"

class Project : public Operator {
public:
    Project(std::unique_ptr<Operator> child, std::vector<std::string> columns);
    std::optional<Block> Next() override;

private:
    std::unique_ptr<Operator> child_;
    std::vector<std::string> columns_;
};
