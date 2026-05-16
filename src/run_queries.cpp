#include <chrono>
#include <iostream>

#include <spdlog/cfg/env.h>
#include <spdlog/spdlog.h>

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
};

static void PrintBlock(const Block& block) {
    for (size_t row = 0; row < block.row_count; ++row) {
        for (size_t col = 0; col < block.names.size(); ++col) {
            std::cout << ToString(block.columns[col]->Get(row)) << '\t';
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

    std::cerr << "Q" << idx << ": " << static_cast<double>(duration) / 1000 << "s\n";
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
            std::cerr << "Q" << query_arg << " not implemented yet\n";
            return 1;
        }
        RunQuery(idx, db);
    }

    return 0;
}
