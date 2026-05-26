#include "operators/order_by.h"

#include <algorithm>
#include <iterator>
#include <queue>
#include <vector>

#include "column.h"

OrderBy::OrderBy(std::unique_ptr<Operator> child, std::vector<SortKey> sort_keys,
                 std::optional<size_t> limit, size_t offset)
    : child_(std::move(child)), sort_keys_(std::move(sort_keys)), limit_(limit), offset_(offset) {
}

std::optional<Block> OrderBy::Next() {
    if (done_) {
        return std::nullopt;
    }
    done_ = true;

    std::vector<std::string> col_names;
    std::vector<Type> col_types;
    std::vector<size_t> key_indices;
    bool schema_set = false;

    auto setup_schema = [&](const Block& block) {
        col_names = block.names;
        for (const auto& col : block.columns) {
            col_types.push_back(col->GetType());
        }
        for (const auto& sk : sort_keys_) {
            key_indices.push_back(std::distance(
                col_names.begin(), std::find(col_names.begin(), col_names.end(), sk.column)));
        }
    };

    auto row_better = [&](const Row& a, const Row& b) -> bool {
        for (size_t i = 0; i < sort_keys_.size(); ++i) {
            size_t idx = key_indices[i];
            if (Less(a[idx], b[idx])) {
                return !sort_keys_[i].descending;
            } else if (Greater(a[idx], b[idx])) {
                return sort_keys_[i].descending;
            }
        }
        return false;
    };

    auto build_result = [&](std::vector<Row>& rows, size_t start, size_t end) -> Block {
        Block result;
        std::vector<std::unique_ptr<Column>> cols;
        for (size_t c = 0; c < col_names.size(); ++c) {
            cols.push_back(MakeColumn(col_types[c]));
        }
        for (size_t i = start; i < end; ++i) {
            for (size_t c = 0; c < cols.size(); ++c) {
                cols[c]->PushBack(rows[i][c]);
            }
        }
        result.row_count = end - start;
        for (size_t c = 0; c < col_names.size(); ++c) {
            result.AddColumn(col_names[c], std::move(cols[c]));
        }
        return result;
    };

    if (!limit_) {
        std::vector<Row> all_rows;
        while (auto block = child_->Next()) {
            if (!schema_set) {
                schema_set = true;
                setup_schema(*block);
            }
            for (size_t row = 0; row < block->row_count; ++row) {
                Row r;
                r.reserve(block->columns.size());
                for (const auto& col : block->columns) {
                    r.push_back(col->Get(row));
                }
                all_rows.push_back(std::move(r));
            }
        }
        if (all_rows.empty()) {
            return std::nullopt;
        }
        std::sort(all_rows.begin(), all_rows.end(), row_better);
        size_t end = all_rows.size();
        return build_result(all_rows, 0, end);
    }

    size_t keep = *limit_ + offset_;
    std::priority_queue<Row, std::vector<Row>, decltype(row_better)> pq(row_better);

    while (auto block = child_->Next()) {
        if (!schema_set) {
            schema_set = true;
            setup_schema(*block);
        }
        for (size_t row = 0; row < block->row_count; ++row) {
            Row r;
            r.reserve(block->columns.size());
            for (const auto& col : block->columns) {
                r.push_back(col->Get(row));
            }
            if (pq.size() < keep) {
                pq.push(std::move(r));
            } else if (row_better(r, pq.top())) {
                pq.pop();
                pq.push(std::move(r));
            }
        }
    }

    if (pq.empty()) {
        return std::nullopt;
    }

    std::vector<Row> rows;
    rows.reserve(pq.size());
    while (!pq.empty()) {
        rows.push_back(pq.top());
        pq.pop();
    }
    std::reverse(rows.begin(), rows.end());

    size_t start = std::min(offset_, rows.size());
    size_t end = std::min(offset_ + *limit_, rows.size());
    return build_result(rows, start, end);
}
