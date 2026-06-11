#include "operators/aggregator.h"

SumAggregator::SumAggregator(std::unique_ptr<Expression> expr) : expr_(std::move(expr)) {
}

void SumAggregator::Accumulate(const Block& block, size_t row_idx) {
    sum_ = Add(sum_, expr_->Evaluate(block, row_idx));
}

Value SumAggregator::Result() const {
    return sum_;
}

void CountAggregator::Accumulate(const Block&, size_t) {
    ++count_;
}

Value CountAggregator::Result() const {
    return int64_t{count_};
}

AvgAggregator::AvgAggregator(std::unique_ptr<Expression> expr) : expr_(std::move(expr)) {
}

void AvgAggregator::Accumulate(const Block& block, size_t row_idx) {
    sum_ = Add(sum_, expr_->Evaluate(block, row_idx));
    ++count_;
}

Value AvgAggregator::Result() const {
    if (count_ == 0) {
        return __int128{0};
    }
    return Div(sum_, Value{int64_t{count_}});
}

CountDistinctAggregator::CountDistinctAggregator(std::unique_ptr<Expression> expr)
    : expr_(std::move(expr)) {
}

void CountDistinctAggregator::Accumulate(const Block& block, size_t row_idx) {
    seen_.insert(expr_->Evaluate(block, row_idx));
}

Value CountDistinctAggregator::Result() const {
    return int64_t{static_cast<int64_t>(seen_.size())};
}

MinAggregator::MinAggregator(std::unique_ptr<Expression> expr) : expr_(std::move(expr)) {
}

void MinAggregator::Accumulate(const Block& block, size_t row_idx) {
    Value val = expr_->Evaluate(block, row_idx);
    if (!min_ || Less(val, *min_)) {
        min_ = val;
    }
}

Value MinAggregator::Result() const {
    return min_.value_or(int64_t{0});
}

MaxAggregator::MaxAggregator(std::unique_ptr<Expression> expr) : expr_(std::move(expr)) {
}

void MaxAggregator::Accumulate(const Block& block, size_t row_idx) {
    Value val = expr_->Evaluate(block, row_idx);
    if (!max_ || Greater(val, *max_)) {
        max_ = val;
    }
}

Value MaxAggregator::Result() const {
    return max_.value_or(int64_t{0});
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
