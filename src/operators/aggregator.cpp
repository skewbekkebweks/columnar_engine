#include "operators/aggregator.h"

SumAggregator::SumAggregator(std::unique_ptr<Expression> expr)
    : expr_(std::move(expr)), sum_(int64_t{0}) {
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

AvgAggregator::AvgAggregator(std::unique_ptr<Expression> expr)
    : expr_(std::move(expr)), sum_(int64_t{0}) {
}

void AvgAggregator::Accumulate(const Block& block, size_t row_idx) {
    sum_ = Add(sum_, expr_->Evaluate(block, row_idx));
    ++count_;
}

Value AvgAggregator::Result() const {
    if (count_ == 0) {
        return int64_t{0};
    }
    return Div(sum_, Value{int64_t{count_}});
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
