#include "operators/limit.h"

#include <algorithm>

#include "column.h"

Limit::Limit(std::unique_ptr<Operator> child, size_t limit, size_t offset)
    : child_(std::move(child)), limit_(limit), offset_(offset) {
}

std::optional<Block> Limit::Next() {
    if (done_) {
        return std::nullopt;
    }

    while (rows_emitted_ < limit_) {
        auto block = child_->Next();
        if (!block) {
            done_ = true;
            return std::nullopt;
        }

        size_t block_rows = block->row_count;

        size_t to_skip =
            (rows_skipped_ < offset_) ? std::min(block_rows, offset_ - rows_skipped_) : 0;
        rows_skipped_ += to_skip;

        size_t start = to_skip;
        size_t remaining = block_rows - start;
        if (remaining == 0) {
            continue;
        }

        size_t to_emit = std::min(remaining, limit_ - rows_emitted_);
        rows_emitted_ += to_emit;
        if (rows_emitted_ >= limit_) {
            done_ = true;
        }

        Block result;
        result.row_count = to_emit;
        for (size_t c = 0; c < block->names.size(); ++c) {
            auto col = MakeColumn(block->columns[c]->GetType());
            for (size_t r = start; r < start + to_emit; ++r) {
                col->PushBack(block->columns[c]->Get(r));
            }
            result.AddColumn(block->names[c], std::move(col));
        }
        return result;
    }

    done_ = true;
    return std::nullopt;
}
