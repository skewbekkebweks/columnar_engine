#include <chrono>
#include <iostream>

#include <spdlog/cfg/env.h>
#include <spdlog/spdlog.h>

#include "operators/aggregate.h"
#include "operators/aggregator.h"
#include "operators/count.h"
#include "operators/expression.h"
#include "operators/filter.h"
#include "operators/operator.h"
#include "operators/scan.h"
#include "schema.h"
#include "value.h"

using QueryGenerator = std::function<std::unique_ptr<Operator>(const std::string&)>;

static const std::vector<QueryGenerator> kQueries = {
    [](const std::string& db) {
        return std::make_unique<Count>(std::make_unique<Scan>(db, Schema()));
    },
    [](const std::string& db) {
        return std::make_unique<Count>(std::make_unique<Filter>(
            std::make_unique<Scan>(db, Schema({{"AdvEngineID", Type::Int64}})),
            Ne(MakeRef("AdvEngineID"), MakeConst(int64_t{0}))));
    },
    [](const std::string& db) {
        std::vector<std::string> names{"SUM(AdvEngineID)", "COUNT(*)", "AVG(ResolutionWidth)"};
        std::vector<std::unique_ptr<Aggregator>> aggs;
        aggs.push_back(MakeSumAgg(MakeRef("AdvEngineID")));
        aggs.push_back(MakeCountAgg());
        aggs.push_back(MakeAvgAgg(MakeRef("ResolutionWidth")));
        return std::make_unique<Aggregate>(
            std::make_unique<Scan>(
                db, Schema({{"AdvEngineID", Type::Int64}, {"ResolutionWidth", Type::Int64}})),
            std::move(names), std::move(aggs));
    },
    [](const std::string& db) {
        std::vector<std::string> names{"AVG(UserID)"};
        std::vector<std::unique_ptr<Aggregator>> aggs;
        aggs.push_back(MakeAvgAgg(MakeRef("UserID")));
        return std::make_unique<Aggregate>(
            std::make_unique<Scan>(db, Schema({{"UserID", Type::Int64}})), std::move(names),
            std::move(aggs));
    },
    [](const std::string& db) {
        std::vector<std::string> names{"COUNT(DISTINCT UserID)"};
        std::vector<std::unique_ptr<Aggregator>> aggs;
        aggs.push_back(MakeCountDistinctAgg(MakeRef("UserID")));
        return std::make_unique<Aggregate>(
            std::make_unique<Scan>(db, Schema({{"UserID", Type::Int64}})), std::move(names),
            std::move(aggs));
    },
    [](const std::string& db) {
        std::vector<std::string> names{"COUNT(DISTINCT SearchPhrase)"};
        std::vector<std::unique_ptr<Aggregator>> aggs;
        aggs.push_back(MakeCountDistinctAgg(MakeRef("SearchPhrase")));
        return std::make_unique<Aggregate>(
            std::make_unique<Scan>(db, Schema({{"SearchPhrase", Type::String}})), std::move(names),
            std::move(aggs));
    },
    [](const std::string& db) {
        std::vector<std::string> names{"MIN(EventDate)", "MAX(EventDate)"};
        std::vector<std::unique_ptr<Aggregator>> aggs;
        aggs.push_back(MakeMinAgg(MakeRef("EventDate")));
        aggs.push_back(MakeMaxAgg(MakeRef("EventDate")));
        return std::make_unique<Aggregate>(
            std::make_unique<Scan>(db, Schema({{"EventDate", Type::Int64}})), std::move(names),
            std::move(aggs));
    },
};

static void PrintBlock(const Block& block) {
    for (size_t row = 0; row < block.row_count; ++row) {
        for (size_t col = 0; col < block.names.size(); ++col) {
            if (col != 0) {
                std::cout << ',';
            }
            std::cout << ToString(block.columns[col]->Get(row));
        }
        std::cout << '\n';
    }
}

static void RunQuery(size_t idx, const std::string& db) {
    auto op = kQueries[idx](db);

    auto start_time = std::chrono::steady_clock::now();
    while (auto block = op->Next()) {
        PrintBlock(*block);
    }
    auto end_time = std::chrono::steady_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    std::cerr << "Q" << std::setfill('0') << std::setw(2) << idx << ": "
              << static_cast<double>(duration) / 1000 << "s\n";
}

int main(int argc, char** argv) {
    spdlog::cfg::load_env_levels();

    if (argc != 3) {
        std::cerr << "usage: " << argv[0] << " <db_file> <query_number|all>\n";
        return 1;
    }

    std::string db = argv[1];
    std::string query_arg = argv[2];

    if (query_arg == "all") {
        auto start_time = std::chrono::steady_clock::now();
        for (size_t i = 0; i < kQueries.size(); ++i) {
            RunQuery(i, db);
        }
        auto end_time = std::chrono::steady_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        std::cerr << "total: " << static_cast<double>(duration) / 1000 << "s\n";
    } else {
        size_t idx = std::stoi(query_arg);
        if (idx >= kQueries.size()) {
            std::cerr << "Q" << std::setfill('0') << std::setw(2) << query_arg
                      << " not implemented yet\n";
            return 1;
        }
        RunQuery(idx, db);
    }

    return 0;
}
