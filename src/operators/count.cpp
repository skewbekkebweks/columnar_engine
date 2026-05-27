#include "operators/count.h"

#include "column.h"

Count::Count(std::unique_ptr<Operator> child, std::string alias)
    : child_(std::move(child)), alias_(std::move(alias)) {
}

std::optional<Block> Count::Next() {
    if (done_) {
        return std::nullopt;
    }
    done_ = true;

    size_t total = 0;
    while (auto chunk = child_->Next()) {
        total += chunk->row_count;
    }

    auto col = MakeColumn(Type::Int64);
    col->PushBack(Value{static_cast<int64_t>(total)});

    Block result;
    result.row_count = 1;
    result.AddColumn(alias_, std::move(col));
    return result;
}
