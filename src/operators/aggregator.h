#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include <ankerl/unordered_dense.h>

#include "column.h"
#include "operators/block.h"
#include "operators/expression.h"
#include "value.h"

class Aggregator {
public:
    virtual ~Aggregator() = default;

    virtual void EnsureGroups(size_t num_groups) = 0;

    virtual void Accumulate(size_t group_id, const Block& block, size_t row_idx) = 0;

    virtual void AccumulateAll(const Block& block);

    virtual std::unique_ptr<Column> Finalize() && = 0;
};

class SumAggregator : public Aggregator {
public:
    explicit SumAggregator(std::unique_ptr<Expression> expr);
    void EnsureGroups(size_t num_groups) override;
    void Accumulate(size_t group_id, const Block& block, size_t row_idx) override;
    void AccumulateAll(const Block& block) override;
    std::unique_ptr<Column> Finalize() && override;

private:
    std::unique_ptr<Expression> expr_;
    std::vector<__int128> sums_;
};

class CountAggregator : public Aggregator {
public:
    void EnsureGroups(size_t num_groups) override;
    void Accumulate(size_t group_id, const Block& block, size_t row_idx) override;
    void AccumulateAll(const Block& block) override;
    std::unique_ptr<Column> Finalize() && override;

private:
    std::vector<int64_t> counts_;
};

class AvgAggregator : public Aggregator {
public:
    explicit AvgAggregator(std::unique_ptr<Expression> expr);
    void EnsureGroups(size_t num_groups) override;
    void Accumulate(size_t group_id, const Block& block, size_t row_idx) override;
    void AccumulateAll(const Block& block) override;
    std::unique_ptr<Column> Finalize() && override;

private:
    std::unique_ptr<Expression> expr_;
    std::vector<__int128> sums_;
    std::vector<int64_t> counts_;
};

class CountDistinctAggregator : public Aggregator {
public:
    explicit CountDistinctAggregator(std::unique_ptr<Expression> expr);
    void EnsureGroups(size_t num_groups) override;
    void Accumulate(size_t group_id, const Block& block, size_t row_idx) override;
    std::unique_ptr<Column> Finalize() && override;

private:
    std::unique_ptr<Expression> expr_;
    std::vector<ankerl::unordered_dense::set<Value, ValueHash>> seen_;
};

class MinAggregator : public Aggregator {
public:
    explicit MinAggregator(std::unique_ptr<Expression> expr);
    void EnsureGroups(size_t num_groups) override;
    void Accumulate(size_t group_id, const Block& block, size_t row_idx) override;
    void AccumulateAll(const Block& block) override;
    std::unique_ptr<Column> Finalize() && override;

private:
    std::unique_ptr<Expression> expr_;
    std::vector<std::optional<Value>> min_;
    Type result_type_ = Type::Int64;
    bool type_set_ = false;
};

class MaxAggregator : public Aggregator {
public:
    explicit MaxAggregator(std::unique_ptr<Expression> expr);
    void EnsureGroups(size_t num_groups) override;
    void Accumulate(size_t group_id, const Block& block, size_t row_idx) override;
    void AccumulateAll(const Block& block) override;
    std::unique_ptr<Column> Finalize() && override;

private:
    std::unique_ptr<Expression> expr_;
    std::vector<std::optional<Value>> max_;
    Type result_type_ = Type::Int64;
    bool type_set_ = false;
};

using AggFactory = std::function<std::unique_ptr<Aggregator>()>;

std::unique_ptr<Aggregator> MakeSumAgg(std::unique_ptr<Expression> expr);
std::unique_ptr<Aggregator> MakeCountAgg();
std::unique_ptr<Aggregator> MakeAvgAgg(std::unique_ptr<Expression> expr);
std::unique_ptr<Aggregator> MakeCountDistinctAgg(std::unique_ptr<Expression> expr);
std::unique_ptr<Aggregator> MakeMinAgg(std::unique_ptr<Expression> expr);
std::unique_ptr<Aggregator> MakeMaxAgg(std::unique_ptr<Expression> expr);
