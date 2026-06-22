#include "operators/filter.h"

#include "column.h"

Filter::Filter(std::unique_ptr<Operator> child, std::unique_ptr<Expression> predicate)
    : child_(std::move(child)), predicate_(std::move(predicate)) {
}

std::optional<Block> Filter::Next() {
    while (auto chunk = child_->Next()) {
        std::vector<size_t> selected;
        for (size_t row = 0; row < chunk->row_count; ++row) {
            if (!IsZero(predicate_->Evaluate(*chunk, row))) {
                selected.push_back(row);
            }
        }

        if (selected.empty()) {
            continue;
        }

        if (selected.size() == chunk->row_count) {
            return chunk;
        }

        Block result;
        result.row_count = selected.size();
        for (size_t j = 0; j < chunk->names.size(); ++j) {
            auto col = MakeColumn(chunk->columns[j]->GetType());
            col->AppendSelected(*chunk->columns[j], selected);
            result.AddColumn(chunk->names[j], std::move(col));
        }
        return result;
    }
    return std::nullopt;
}
