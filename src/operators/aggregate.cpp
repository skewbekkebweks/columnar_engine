#include "operators/aggregate.h"

#include <utility>

#include "column.h"

Aggregate::Aggregate(std::unique_ptr<Operator> child, std::vector<std::string> names,
                     std::vector<std::unique_ptr<Aggregator>> aggs)
    : child_(std::move(child)), names_(std::move(names)), aggs_(std::move(aggs)) {
}

std::optional<Block> Aggregate::Next() {
    if (done_) {
        return std::nullopt;
    }
    done_ = true;

    for (auto& agg : aggs_) {
        agg->EnsureGroups(1);
    }

    while (auto chunk = child_->Next()) {
        for (auto& agg : aggs_) {
            agg->AccumulateAll(*chunk);
        }
    }

    Block result;
    result.row_count = 1;
    for (size_t i = 0; i < aggs_.size(); ++i) {
        result.AddColumn(names_[i], std::move(*aggs_[i]).Finalize());
    }
    return result;
}
