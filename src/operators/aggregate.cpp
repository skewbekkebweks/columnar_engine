#include "operators/aggregate.h"

#include "column.h"
#include "type.h"

Aggregate::Aggregate(std::unique_ptr<Operator> child, std::vector<std::string> names,
                     std::vector<std::unique_ptr<Aggregator>> aggs)
    : child_(std::move(child)), names_(std::move(names)), aggs_(std::move(aggs)) {
}

std::optional<Block> Aggregate::Next() {
    if (done_) {
        return std::nullopt;
    }
    done_ = true;

    while (auto chunk = child_->Next()) {
        for (size_t row = 0; row < chunk->row_count; ++row) {
            for (auto& agg : aggs_) {
                agg->Accumulate(*chunk, row);
            }
        }
    }

    Block result;
    result.row_count = 1;
    for (size_t i = 0; i < aggs_.size(); ++i) {
        Value val = aggs_[i]->Result();
        Type type = std::holds_alternative<int64_t>(val) ? Type::Int64 : Type::String;
        auto col = MakeColumn(type);
        col->PushBack(val);
        result.AddColumn(names_[i], std::move(col));
    }
    return result;
}
