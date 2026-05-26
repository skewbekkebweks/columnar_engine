#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "operators/block.h"
#include "operators/operator.h"
#include "type.h"
#include "value.h"

struct SortKey {
    std::string column;
    bool descending;
};

class OrderBy : public Operator {
public:
    OrderBy(std::unique_ptr<Operator> child, std::vector<SortKey> sort_keys,
            std::optional<size_t> limit = std::nullopt, size_t offset = 0);
    std::optional<Block> Next() override;

private:
    using Row = std::vector<Value>;

    std::unique_ptr<Operator> child_;
    std::vector<SortKey> sort_keys_;
    std::optional<size_t> limit_;
    size_t offset_;
    bool done_ = false;
};
