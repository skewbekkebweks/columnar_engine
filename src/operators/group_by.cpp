#include "operators/group_by.h"

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

    using AggList = std::vector<std::unique_ptr<Aggregator>>;

    auto make_aggs = [this]() {
        AggList aggs;
        aggs.reserve(agg_factories_.size());
        for (auto& factory : agg_factories_) {
            aggs.push_back(factory());
        }
        return aggs;
    };

    std::vector<std::unique_ptr<Column>> key_cols;
    std::vector<std::unique_ptr<Column>> agg_cols;
    size_t group_count = 0;

    auto emit_group = [&](const Value* key, const AggList& aggs) {
        if (group_count == 0) {
            for (size_t k = 0; k < key_names_.size(); ++k) {
                key_cols.push_back(MakeColumn(TypeOf(key[k])));
            }
            for (size_t a = 0; a < agg_names_.size(); ++a) {
                agg_cols.push_back(MakeColumn(TypeOf(aggs[a]->Result())));
            }
        }
        for (size_t k = 0; k < key_names_.size(); ++k) {
            key_cols[k]->PushBack(key[k]);
        }
        for (size_t a = 0; a < agg_names_.size(); ++a) {
            agg_cols[a]->PushBack(aggs[a]->Result());
        }
        ++group_count;
    };

    if (key_exprs_.size() == 1) {
        ankerl::unordered_dense::map<Value, AggList, ValueHash> groups;
        while (auto block = child_->Next()) {
            for (size_t row = 0; row < block->row_count; ++row) {
                auto [it, inserted] = groups.try_emplace(key_exprs_[0]->Evaluate(*block, row));
                if (inserted) {
                    it->second = make_aggs();
                }
                for (auto& agg : it->second) {
                    agg->Accumulate(*block, row);
                }
            }
        }
        for (const auto& [key, aggs] : groups) {
            emit_group(&key, aggs);
        }
    } else {
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
        ankerl::unordered_dense::map<Key, AggList, KeyHash> groups;
        Key key;
        while (auto block = child_->Next()) {
            for (size_t row = 0; row < block->row_count; ++row) {
                key.clear();
                for (auto& expr : key_exprs_) {
                    key.push_back(expr->Evaluate(*block, row));
                }
                auto [it, inserted] = groups.try_emplace(std::move(key));
                if (inserted) {
                    it->second = make_aggs();
                }
                for (auto& agg : it->second) {
                    agg->Accumulate(*block, row);
                }
            }
        }
        for (const auto& [key_vals, aggs] : groups) {
            emit_group(key_vals.data(), aggs);
        }
    }

    if (group_count == 0) {
        return std::nullopt;
    }

    Block result;
    result.row_count = group_count;
    for (size_t k = 0; k < key_names_.size(); ++k) {
        result.AddColumn(key_names_[k], std::move(key_cols[k]));
    }
    for (size_t a = 0; a < agg_names_.size(); ++a) {
        result.AddColumn(agg_names_[a], std::move(agg_cols[a]));
    }

    return result;
}
