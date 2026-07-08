#include "operators/group_by.h"

#include <cstdint>
#include <utility>
#include <vector>

#include <ankerl/unordered_dense.h>

#include "column.h"
#include "value.h"

GroupBy::GroupBy(std::unique_ptr<Operator> child,
                 std::vector<std::unique_ptr<Expression>> key_exprs,
                 std::vector<std::string> key_names, std::vector<std::string> agg_names,
                 std::vector<AggFactory> agg_factories)
    : child_(std::move(child)),
      key_exprs_(std::move(key_exprs)),
      key_names_(std::move(key_names)),
      agg_names_(std::move(agg_names)),
      agg_factories_(std::move(agg_factories)) {
}

std::optional<Block> GroupBy::Next() {
    if (done_) {
        return std::nullopt;
    }
    done_ = true;

    std::vector<std::unique_ptr<Aggregator>> aggs;
    aggs.reserve(agg_factories_.size());
    for (auto& factory : agg_factories_) {
        aggs.push_back(factory());
    }

    auto accumulate_row = [&](uint32_t gid, const Block& block, size_t row) {
        for (auto& agg : aggs) {
            agg->Accumulate(gid, block, row);
        }
    };

    auto build_result = [&](auto& key_cols) -> Block {
        Block result;
        result.row_count = key_cols.empty() ? 0 : key_cols[0]->Size();
        for (size_t k = 0; k < key_names_.size(); ++k) {
            result.AddColumn(key_names_[k], std::move(key_cols[k]));
        }
        for (size_t a = 0; a < agg_names_.size(); ++a) {
            result.AddColumn(agg_names_[a], std::move(*aggs[a]).Finalize());
        }
        return result;
    };

    if (key_exprs_.size() == 1) {
        ankerl::unordered_dense::map<Value, uint32_t, ValueHash> groups;

        std::vector<Value> key_values;

        Type key_type = Type::Int64;
        bool type_set = false;

        while (auto block = child_->Next()) {
            if (!type_set) {
                key_type = key_exprs_[0]->ResultType(*block);
                type_set = true;
            }
            for (size_t row = 0; row < block->row_count; ++row) {
                Value key = key_exprs_[0]->Evaluate(*block, row);
                auto [it, inserted] =
                    groups.try_emplace(key, static_cast<uint32_t>(key_values.size()));
                if (inserted) {
                    key_values.push_back(std::move(key));
                    for (auto& agg : aggs) {
                        agg->EnsureGroups(key_values.size());
                    }
                }
                accumulate_row(it->second, *block, row);
            }
        }

        if (key_values.empty()) {
            return std::nullopt;
        }

        std::vector<std::unique_ptr<Column>> key_cols;
        key_cols.push_back(MakeColumn(key_type));
        for (const auto& v : key_values) {
            key_cols[0]->PushBack(v);
        }
        return build_result(key_cols);
    }

    using Key = std::vector<Value>;
    struct KeyHash {
        size_t operator()(const Key& key) const {
            size_t h = 0;
            for (const auto& v : key) {
                h = h * 1000000007ULL + ValueHash{}(v);
            }
            return h;
        }
    };

    ankerl::unordered_dense::map<Key, uint32_t, KeyHash> groups;

    std::vector<Key> key_rows;
    Key key;

    std::vector<Type> key_types(key_names_.size(), Type::Int64);
    bool type_set = false;

    while (auto block = child_->Next()) {
        if (!type_set) {
            for (size_t k = 0; k < key_exprs_.size(); ++k) {
                key_types[k] = key_exprs_[k]->ResultType(*block);
            }
            type_set = true;
        }
        for (size_t row = 0; row < block->row_count; ++row) {
            key.clear();
            for (auto& expr : key_exprs_) {
                key.push_back(expr->Evaluate(*block, row));
            }
            auto [it, inserted] = groups.try_emplace(key, static_cast<uint32_t>(key_rows.size()));
            if (inserted) {
                key_rows.push_back(std::move(key));
                for (auto& agg : aggs) {
                    agg->EnsureGroups(key_rows.size());
                }
            }
            accumulate_row(it->second, *block, row);
        }
    }

    if (key_rows.empty()) {
        return std::nullopt;
    }

    std::vector<std::unique_ptr<Column>> key_cols;
    for (size_t k = 0; k < key_names_.size(); ++k) {
        key_cols.push_back(MakeColumn(key_types[k]));
        for (const auto& kr : key_rows) {
            key_cols[k]->PushBack(kr[k]);
        }
    }
    return build_result(key_cols);
}
