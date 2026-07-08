#include "operators/aggregator.h"

namespace {

const int64_t* Int64ColumnData(const Expression& expr, const Block& block) {
    const std::string* name = expr.AsColumnName();
    if (name == nullptr || !block.HasColumn(*name)) {
        return nullptr;
    }
    return block.GetColumn(*name).AsInt64Data();
}

}  // namespace

void Aggregator::AccumulateAll(const Block& block) {
    for (size_t row = 0; row < block.row_count; ++row) {
        Accumulate(0, block, row);
    }
}

SumAggregator::SumAggregator(std::unique_ptr<Expression> expr) : expr_(std::move(expr)) {
}

void SumAggregator::EnsureGroups(size_t num_groups) {
    sums_.resize(num_groups, 0);
}

void SumAggregator::Accumulate(size_t group_id, const Block& block, size_t row_idx) {
    sums_[group_id] += ToInt128(expr_->Evaluate(block, row_idx));
}

void SumAggregator::AccumulateAll(const Block& block) {
    if (const int64_t* data = Int64ColumnData(*expr_, block)) {
        __int128 sum = 0;
        for (size_t i = 0; i < block.row_count; ++i) {
            sum += data[i];
        }
        sums_[0] += sum;
        return;
    }
    Aggregator::AccumulateAll(block);
}

std::unique_ptr<Column> SumAggregator::Finalize() && {
    auto col = MakeColumn(Type::Int128);
    for (__int128 s : sums_) {
        col->PushBack(Value{s});
    }
    return col;
}

void CountAggregator::EnsureGroups(size_t num_groups) {
    counts_.resize(num_groups, 0);
}

void CountAggregator::Accumulate(size_t group_id, const Block&, size_t) {
    ++counts_[group_id];
}

void CountAggregator::AccumulateAll(const Block& block) {
    counts_[0] += static_cast<int64_t>(block.row_count);
}

std::unique_ptr<Column> CountAggregator::Finalize() && {
    auto col = MakeColumn(Type::Int64);
    for (int64_t c : counts_) {
        col->PushBack(Value{c});
    }
    return col;
}

AvgAggregator::AvgAggregator(std::unique_ptr<Expression> expr) : expr_(std::move(expr)) {
}

void AvgAggregator::EnsureGroups(size_t num_groups) {
    sums_.resize(num_groups, 0);
    counts_.resize(num_groups, 0);
}

void AvgAggregator::Accumulate(size_t group_id, const Block& block, size_t row_idx) {
    sums_[group_id] += ToInt128(expr_->Evaluate(block, row_idx));
    ++counts_[group_id];
}

void AvgAggregator::AccumulateAll(const Block& block) {
    if (const int64_t* data = Int64ColumnData(*expr_, block)) {
        __int128 sum = 0;
        for (size_t i = 0; i < block.row_count; ++i) {
            sum += data[i];
        }
        sums_[0] += sum;
        counts_[0] += static_cast<int64_t>(block.row_count);
        return;
    }
    Aggregator::AccumulateAll(block);
}

std::unique_ptr<Column> AvgAggregator::Finalize() && {
    auto col = MakeColumn(Type::Int128);
    for (size_t i = 0; i < sums_.size(); ++i) {
        __int128 avg = counts_[i] == 0 ? __int128{0} : sums_[i] / counts_[i];
        col->PushBack(Value{avg});
    }
    return col;
}

CountDistinctAggregator::CountDistinctAggregator(std::unique_ptr<Expression> expr)
    : expr_(std::move(expr)) {
}

void CountDistinctAggregator::EnsureGroups(size_t num_groups) {
    seen_.resize(num_groups);
}

void CountDistinctAggregator::Accumulate(size_t group_id, const Block& block, size_t row_idx) {
    seen_[group_id].insert(expr_->Evaluate(block, row_idx));
}

std::unique_ptr<Column> CountDistinctAggregator::Finalize() && {
    auto col = MakeColumn(Type::Int64);
    for (const auto& s : seen_) {
        col->PushBack(Value{static_cast<int64_t>(s.size())});
    }
    return col;
}

MinAggregator::MinAggregator(std::unique_ptr<Expression> expr) : expr_(std::move(expr)) {
}

void MinAggregator::EnsureGroups(size_t num_groups) {
    min_.resize(num_groups);
}

void MinAggregator::Accumulate(size_t group_id, const Block& block, size_t row_idx) {
    if (!type_set_) {
        result_type_ = expr_->ResultType(block);
        type_set_ = true;
    }
    Value val = expr_->Evaluate(block, row_idx);
    if (!min_[group_id] || Less(val, *min_[group_id])) {
        min_[group_id] = std::move(val);
    }
}

void MinAggregator::AccumulateAll(const Block& block) {
    const std::string* name = expr_->AsColumnName();
    if (name != nullptr && block.HasColumn(*name) && block.row_count > 0) {
        if (!type_set_) {
            result_type_ = expr_->ResultType(block);
            type_set_ = true;
        }
        int64_t min, max;
        if (block.GetColumn(*name).TryGetMinMax(min, max)) {
            Value val{min};
            if (!min_[0] || Less(val, *min_[0])) {
                min_[0] = std::move(val);
            }
            return;
        }
    }
    Aggregator::AccumulateAll(block);
}

std::unique_ptr<Column> MinAggregator::Finalize() && {
    auto col = MakeColumn(result_type_);
    for (const auto& v : min_) {
        col->PushBack(v ? *v : Value{int64_t{0}});
    }
    return col;
}

MaxAggregator::MaxAggregator(std::unique_ptr<Expression> expr) : expr_(std::move(expr)) {
}

void MaxAggregator::EnsureGroups(size_t num_groups) {
    max_.resize(num_groups);
}

void MaxAggregator::Accumulate(size_t group_id, const Block& block, size_t row_idx) {
    if (!type_set_) {
        result_type_ = expr_->ResultType(block);
        type_set_ = true;
    }
    Value val = expr_->Evaluate(block, row_idx);
    if (!max_[group_id] || Greater(val, *max_[group_id])) {
        max_[group_id] = std::move(val);
    }
}

void MaxAggregator::AccumulateAll(const Block& block) {
    const std::string* name = expr_->AsColumnName();
    if (name != nullptr && block.HasColumn(*name) && block.row_count > 0) {
        if (!type_set_) {
            result_type_ = expr_->ResultType(block);
            type_set_ = true;
        }
        int64_t min, max;
        if (block.GetColumn(*name).TryGetMinMax(min, max)) {
            Value val{max};
            if (!max_[0] || Greater(val, *max_[0])) {
                max_[0] = std::move(val);
            }
            return;
        }
    }
    Aggregator::AccumulateAll(block);
}

std::unique_ptr<Column> MaxAggregator::Finalize() && {
    auto col = MakeColumn(result_type_);
    for (const auto& v : max_) {
        col->PushBack(v ? *v : Value{int64_t{0}});
    }
    return col;
}

std::unique_ptr<Aggregator> MakeSumAgg(std::unique_ptr<Expression> expr) {
    return std::make_unique<SumAggregator>(std::move(expr));
}

std::unique_ptr<Aggregator> MakeCountAgg() {
    return std::make_unique<CountAggregator>();
}

std::unique_ptr<Aggregator> MakeAvgAgg(std::unique_ptr<Expression> expr) {
    return std::make_unique<AvgAggregator>(std::move(expr));
}

std::unique_ptr<Aggregator> MakeCountDistinctAgg(std::unique_ptr<Expression> expr) {
    return std::make_unique<CountDistinctAggregator>(std::move(expr));
}

std::unique_ptr<Aggregator> MakeMinAgg(std::unique_ptr<Expression> expr) {
    return std::make_unique<MinAggregator>(std::move(expr));
}

std::unique_ptr<Aggregator> MakeMaxAgg(std::unique_ptr<Expression> expr) {
    return std::make_unique<MaxAggregator>(std::move(expr));
}
