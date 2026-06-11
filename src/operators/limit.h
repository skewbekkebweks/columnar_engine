#pragma once

#include <memory>
#include <optional>
#include "operators/block.h"
#include "operators/operator.h"

class Limit : public Operator {
public:
    Limit(std::unique_ptr<Operator> child, size_t limit, size_t offset = 0);
    std::optional<Block> Next() override;

private:
    std::unique_ptr<Operator> child_;
    size_t limit_;
    size_t offset_;
    size_t rows_skipped_ = 0;
    size_t rows_emitted_ = 0;
    bool done_ = false;
};
