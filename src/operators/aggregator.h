#pragma once

#include <memory>
#include <optional>
#include <unordered_set>
#include "operators/block.h"
#include "operators/expression.h"
#include "value.h"

class Aggregator {
public:
    virtual ~Aggregator() = default;
    virtual void Accumulate(const Block& block, size_t row_idx) = 0;
    virtual Value Result() const = 0;
};

class SumAggregator : public Aggregator {
public:
    explicit SumAggregator(std::unique_ptr<Expression> expr);
    void Accumulate(const Block& block, size_t row_idx) override;
    Value Result() const override;

private:
    std::unique_ptr<Expression> expr_;
    Value sum_ = __int128{0};
};

class CountAggregator : public Aggregator {
public:
    void Accumulate(const Block& block, size_t row_idx) override;
    Value Result() const override;

private:
    int64_t count_ = 0;
};

class AvgAggregator : public Aggregator {
public:
    explicit AvgAggregator(std::unique_ptr<Expression> expr);
    void Accumulate(const Block& block, size_t row_idx) override;
    Value Result() const override;

private:
    std::unique_ptr<Expression> expr_;
    Value sum_ = __int128{0};
    int64_t count_ = 0;
};

class CountDistinctAggregator : public Aggregator {
public:
    explicit CountDistinctAggregator(std::unique_ptr<Expression> expr);
    void Accumulate(const Block& block, size_t row_idx) override;
    Value Result() const override;

private:
    std::unique_ptr<Expression> expr_;
    std::unordered_set<Value, ValueHash> seen_;
};

class MinAggregator : public Aggregator {
public:
    explicit MinAggregator(std::unique_ptr<Expression> expr);
    void Accumulate(const Block& block, size_t row_idx) override;
    Value Result() const override;

private:
    std::unique_ptr<Expression> expr_;
    std::optional<Value> min_;
};

class MaxAggregator : public Aggregator {
public:
    explicit MaxAggregator(std::unique_ptr<Expression> expr);
    void Accumulate(const Block& block, size_t row_idx) override;
    Value Result() const override;

private:
    std::unique_ptr<Expression> expr_;
    std::optional<Value> max_;
};

std::unique_ptr<Aggregator> MakeSumAgg(std::unique_ptr<Expression> expr);
std::unique_ptr<Aggregator> MakeCountAgg();
std::unique_ptr<Aggregator> MakeAvgAgg(std::unique_ptr<Expression> expr);
std::unique_ptr<Aggregator> MakeCountDistinctAgg(std::unique_ptr<Expression> expr);
std::unique_ptr<Aggregator> MakeMinAgg(std::unique_ptr<Expression> expr);
std::unique_ptr<Aggregator> MakeMaxAgg(std::unique_ptr<Expression> expr);
