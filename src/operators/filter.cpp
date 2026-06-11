#include "operators/filter.h"

#include "column.h"

Filter::Filter(std::unique_ptr<Operator> child, std::unique_ptr<Expression> predicate)
    : child_(std::move(child)), predicate_(std::move(predicate)) {
}

std::optional<Block> Filter::Next() {
    while (auto chunk = child_->Next()) {
        Block result;
        for (size_t j = 0; j < chunk->names.size(); ++j) {
            result.AddColumn(chunk->names[j], MakeColumn(chunk->columns[j]->GetType()));
        }

        for (size_t row = 0; row < chunk->row_count; ++row) {
            if (!IsZero(predicate_->Evaluate(*chunk, row))) {
                for (size_t j = 0; j < chunk->columns.size(); ++j) {
                    result.columns[j]->PushBack(chunk->columns[j]->Get(row));
                }
                ++result.row_count;
            }
        }

        if (result.row_count > 0) {
            return result;
        }
    }
    return std::nullopt;
}
