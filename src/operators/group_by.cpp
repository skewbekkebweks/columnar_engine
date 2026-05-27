#include "operators/group_by.h"

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

    ankerl::unordered_dense::map<Key, std::vector<std::unique_ptr<Aggregator>>, KeyHash> groups;
    std::vector<Key> key_order;

    while (auto block = child_->Next()) {
        for (size_t row = 0; row < block->row_count; ++row) {
            Key key;
            key.reserve(key_exprs_.size());
            for (auto& expr : key_exprs_) {
                key.push_back(expr->Evaluate(*block, row));
            }
            auto it = groups.find(key);
            if (it == groups.end()) {
                key_order.push_back(key);
                std::vector<std::unique_ptr<Aggregator>> aggs;
                aggs.reserve(agg_factories_.size());
                for (auto& factory : agg_factories_) {
                    aggs.push_back(factory());
                }
                it = groups.emplace(key, std::move(aggs)).first;
            }
            for (auto& agg : it->second) {
                agg->Accumulate(*block, row);
            }
        }
    }

    if (key_order.empty()) {
        return std::nullopt;
    }

    const Key& first_key = key_order[0];
    auto& first_aggs = groups.at(first_key);

    std::vector<std::unique_ptr<Column>> key_cols;
    for (size_t k = 0; k < key_names_.size(); ++k) {
        key_cols.push_back(MakeColumn(TypeOf(first_key[k])));
    }
    std::vector<std::unique_ptr<Column>> agg_cols;
    for (size_t a = 0; a < agg_names_.size(); ++a) {
        agg_cols.push_back(MakeColumn(TypeOf(first_aggs[a]->Result())));
    }

    for (const Key& key : key_order) {
        auto& aggs = groups.at(key);
        for (size_t k = 0; k < key_names_.size(); ++k) {
            key_cols[k]->PushBack(key[k]);
        }
        for (size_t a = 0; a < agg_names_.size(); ++a) {
            agg_cols[a]->PushBack(aggs[a]->Result());
        }
    }

    Block result;
    result.row_count = key_order.size();
    for (size_t k = 0; k < key_names_.size(); ++k) {
        result.AddColumn(key_names_[k], std::move(key_cols[k]));
    }
    for (size_t a = 0; a < agg_names_.size(); ++a) {
        result.AddColumn(agg_names_[a], std::move(agg_cols[a]));
    }

    return result;
}
