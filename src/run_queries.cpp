#include <chrono>
#include <iostream>

#include <spdlog/cfg/env.h>
#include <spdlog/spdlog.h>

#include "operators/aggregate.h"
#include "operators/aggregator.h"
#include "operators/count.h"
#include "operators/expression.h"
#include "operators/filter.h"
#include "operators/group_by.h"
#include "operators/limit.h"
#include "operators/operator.h"
#include "operators/order_by.h"
#include "operators/project.h"
#include "operators/scan.h"
#include "schema.h"
#include "value.h"

using QueryGenerator = std::function<std::unique_ptr<Operator>(const std::string&)>;

static const std::vector<QueryGenerator> kQueries = {
    // Q00
    [](const std::string& db) {
        return std::make_unique<Count>(std::make_unique<Scan>(db, Schema()));
    },
    // Q01
    [](const std::string& db) {
        return std::make_unique<Count>(std::make_unique<Filter>(
            std::make_unique<Scan>(db, Schema({{"AdvEngineID", Type::Int64}})),
            Ne(MakeRef("AdvEngineID"), MakeConst(int64_t{0}))));
    },
    // Q02
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
    // Q03
    [](const std::string& db) {
        std::vector<std::string> names{"AVG(UserID)"};
        std::vector<std::unique_ptr<Aggregator>> aggs;
        aggs.push_back(MakeAvgAgg(MakeRef("UserID")));
        return std::make_unique<Aggregate>(
            std::make_unique<Scan>(db, Schema({{"UserID", Type::Int64}})), std::move(names),
            std::move(aggs));
    },
    // Q04
    [](const std::string& db) {
        std::vector<std::string> names{"COUNT(DISTINCT UserID)"};
        std::vector<std::unique_ptr<Aggregator>> aggs;
        aggs.push_back(MakeCountDistinctAgg(MakeRef("UserID")));
        return std::make_unique<Aggregate>(
            std::make_unique<Scan>(db, Schema({{"UserID", Type::Int64}})), std::move(names),
            std::move(aggs));
    },
    // Q05
    [](const std::string& db) {
        std::vector<std::string> names{"COUNT(DISTINCT SearchPhrase)"};
        std::vector<std::unique_ptr<Aggregator>> aggs;
        aggs.push_back(MakeCountDistinctAgg(MakeRef("SearchPhrase")));
        return std::make_unique<Aggregate>(
            std::make_unique<Scan>(db, Schema({{"SearchPhrase", Type::String}})), std::move(names),
            std::move(aggs));
    },
    // Q06
    [](const std::string& db) {
        std::vector<std::string> names{"MIN(EventDate)", "MAX(EventDate)"};
        std::vector<std::unique_ptr<Aggregator>> aggs;
        aggs.push_back(MakeMinAgg(MakeRef("EventDate")));
        aggs.push_back(MakeMaxAgg(MakeRef("EventDate")));
        return std::make_unique<Aggregate>(
            std::make_unique<Scan>(db, Schema({{"EventDate", Type::Date}})), std::move(names),
            std::move(aggs));
    },
    // Q07
    [](const std::string& db) {
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(MakeRef("AdvEngineID"));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeCountAgg(); });
        return std::make_unique<OrderBy>(
            std::make_unique<GroupBy>(
                std::make_unique<Filter>(
                    std::make_unique<Scan>(db, Schema({{"AdvEngineID", Type::Int64}})),
                    Ne(MakeRef("AdvEngineID"), MakeConst(int64_t{0}))),
                std::move(key_exprs), std::vector<std::string>{"AdvEngineID"},
                std::vector<std::string>{"COUNT(*)"}, std::move(agg_factories)),
            std::vector<SortKey>{{"COUNT(*)", true}});
    },
    // Q08
    [](const std::string& db) {
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(MakeRef("RegionID"));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeCountDistinctAgg(MakeRef("UserID")); });
        return std::make_unique<OrderBy>(
            std::make_unique<GroupBy>(
                std::make_unique<Scan>(
                    db, Schema({{"RegionID", Type::Int64}, {"UserID", Type::Int64}})),
                std::move(key_exprs), std::vector<std::string>{"RegionID"},
                std::vector<std::string>{"COUNT(DISTINCT UserID)"}, std::move(agg_factories)),
            std::vector<SortKey>{{"COUNT(DISTINCT UserID)", true}}, 10);
    },
    // Q09
    [](const std::string& db) {
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(MakeRef("RegionID"));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeSumAgg(MakeRef("AdvEngineID")); });
        agg_factories.push_back([]() { return MakeCountAgg(); });
        agg_factories.push_back([]() { return MakeAvgAgg(MakeRef("ResolutionWidth")); });
        agg_factories.push_back([]() { return MakeCountDistinctAgg(MakeRef("UserID")); });
        return std::make_unique<OrderBy>(
            std::make_unique<GroupBy>(
                std::make_unique<Scan>(db, Schema({{"RegionID", Type::Int64},
                                                   {"AdvEngineID", Type::Int64},
                                                   {"ResolutionWidth", Type::Int64},
                                                   {"UserID", Type::Int64}})),
                std::move(key_exprs), std::vector<std::string>{"RegionID"},
                std::vector<std::string>{"SUM(AdvEngineID)", "COUNT(*)", "AVG(ResolutionWidth)",
                                         "COUNT(DISTINCT UserID)"},
                std::move(agg_factories)),
            std::vector<SortKey>{{"COUNT(*)", true}}, 10);
    },
    // Q10
    [](const std::string& db) {
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(MakeRef("MobilePhoneModel"));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeCountDistinctAgg(MakeRef("UserID")); });
        return std::make_unique<OrderBy>(
            std::make_unique<GroupBy>(
                std::make_unique<Filter>(
                    std::make_unique<Scan>(
                        db, Schema({{"MobilePhoneModel", Type::String}, {"UserID", Type::Int64}})),
                    Ne(MakeRef("MobilePhoneModel"), MakeConst(std::string("")))),
                std::move(key_exprs), std::vector<std::string>{"MobilePhoneModel"},
                std::vector<std::string>{"COUNT(DISTINCT UserID)"}, std::move(agg_factories)),
            std::vector<SortKey>{{"COUNT(DISTINCT UserID)", true}}, 10);
    },
    // Q11
    [](const std::string& db) {
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(MakeRef("MobilePhone"));
        key_exprs.push_back(MakeRef("MobilePhoneModel"));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeCountDistinctAgg(MakeRef("UserID")); });
        return std::make_unique<OrderBy>(
            std::make_unique<GroupBy>(
                std::make_unique<Filter>(
                    std::make_unique<Scan>(db, Schema({{"MobilePhone", Type::Int64},
                                                       {"MobilePhoneModel", Type::String},
                                                       {"UserID", Type::Int64}})),
                    Ne(MakeRef("MobilePhoneModel"), MakeConst(std::string("")))),
                std::move(key_exprs), std::vector<std::string>{"MobilePhone", "MobilePhoneModel"},
                std::vector<std::string>{"COUNT(DISTINCT UserID)"}, std::move(agg_factories)),
            std::vector<SortKey>{{"COUNT(DISTINCT UserID)", true}}, 10);
    },
    // Q12
    [](const std::string& db) {
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(MakeRef("SearchPhrase"));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeCountAgg(); });
        return std::make_unique<OrderBy>(
            std::make_unique<GroupBy>(
                std::make_unique<Filter>(
                    std::make_unique<Scan>(db, Schema({{"SearchPhrase", Type::String}})),
                    Ne(MakeRef("SearchPhrase"), MakeConst(std::string("")))),
                std::move(key_exprs), std::vector<std::string>{"SearchPhrase"},
                std::vector<std::string>{"COUNT(*)"}, std::move(agg_factories)),
            std::vector<SortKey>{{"COUNT(*)", true}}, 10);
    },
    // Q13
    [](const std::string& db) {
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(MakeRef("SearchPhrase"));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeCountDistinctAgg(MakeRef("UserID")); });
        return std::make_unique<OrderBy>(
            std::make_unique<GroupBy>(
                std::make_unique<Filter>(
                    std::make_unique<Scan>(
                        db, Schema({{"SearchPhrase", Type::String}, {"UserID", Type::Int64}})),
                    Ne(MakeRef("SearchPhrase"), MakeConst(std::string("")))),
                std::move(key_exprs), std::vector<std::string>{"SearchPhrase"},
                std::vector<std::string>{"COUNT(DISTINCT UserID)"}, std::move(agg_factories)),
            std::vector<SortKey>{{"COUNT(DISTINCT UserID)", true}}, 10);
    },
    // Q14
    [](const std::string& db) {
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(MakeRef("SearchEngineID"));
        key_exprs.push_back(MakeRef("SearchPhrase"));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeCountAgg(); });
        return std::make_unique<OrderBy>(
            std::make_unique<GroupBy>(
                std::make_unique<Filter>(
                    std::make_unique<Scan>(db, Schema({{"SearchEngineID", Type::Int64},
                                                       {"SearchPhrase", Type::String}})),
                    Ne(MakeRef("SearchPhrase"), MakeConst(std::string("")))),
                std::move(key_exprs), std::vector<std::string>{"SearchEngineID", "SearchPhrase"},
                std::vector<std::string>{"COUNT(*)"}, std::move(agg_factories)),
            std::vector<SortKey>{{"COUNT(*)", true}}, 10);
    },
    // Q15
    [](const std::string& db) {
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(MakeRef("UserID"));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeCountAgg(); });
        return std::make_unique<OrderBy>(
            std::make_unique<GroupBy>(std::make_unique<Scan>(db, Schema({{"UserID", Type::Int64}})),
                                      std::move(key_exprs), std::vector<std::string>{"UserID"},
                                      std::vector<std::string>{"COUNT(*)"},
                                      std::move(agg_factories)),
            std::vector<SortKey>{{"COUNT(*)", true}}, 10);
    },
    // Q16
    [](const std::string& db) {
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(MakeRef("UserID"));
        key_exprs.push_back(MakeRef("SearchPhrase"));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeCountAgg(); });
        return std::make_unique<OrderBy>(
            std::make_unique<GroupBy>(
                std::make_unique<Scan>(
                    db, Schema({{"UserID", Type::Int64}, {"SearchPhrase", Type::String}})),
                std::move(key_exprs), std::vector<std::string>{"UserID", "SearchPhrase"},
                std::vector<std::string>{"COUNT(*)"}, std::move(agg_factories)),
            std::vector<SortKey>{{"COUNT(*)", true}}, 10);
    },
    // Q17
    [](const std::string& db) {
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(MakeRef("UserID"));
        key_exprs.push_back(MakeRef("SearchPhrase"));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeCountAgg(); });
        return std::make_unique<Limit>(
            std::make_unique<GroupBy>(
                std::make_unique<Scan>(
                    db, Schema({{"UserID", Type::Int64}, {"SearchPhrase", Type::String}})),
                std::move(key_exprs), std::vector<std::string>{"UserID", "SearchPhrase"},
                std::vector<std::string>{"COUNT(*)"}, std::move(agg_factories)),
            10);
    },
    // Q18
    [](const std::string& db) {
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(MakeRef("UserID"));
        key_exprs.push_back(ExtractMinute(MakeRef("EventTime")));
        key_exprs.push_back(MakeRef("SearchPhrase"));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeCountAgg(); });
        return std::make_unique<OrderBy>(
            std::make_unique<GroupBy>(
                std::make_unique<Scan>(db, Schema({{"UserID", Type::Int64},
                                                   {"EventTime", Type::Timestamp},
                                                   {"SearchPhrase", Type::String}})),
                std::move(key_exprs), std::vector<std::string>{"UserID", "m", "SearchPhrase"},
                std::vector<std::string>{"COUNT(*)"}, std::move(agg_factories)),
            std::vector<SortKey>{{"COUNT(*)", true}}, 10);
    },
    // Q19
    [](const std::string& db) {
        return std::make_unique<Filter>(
            std::make_unique<Scan>(db, Schema({{"UserID", Type::Int64}})),
            Eq(MakeRef("UserID"), MakeConst(int64_t{435090932899640449})));
    },
    // Q20
    [](const std::string& db) {
        return std::make_unique<Count>(
            std::make_unique<Filter>(std::make_unique<Scan>(db, Schema({{"URL", Type::String}})),
                                     Like(MakeRef("URL"), "%google%")));
    },
    // Q21
    [](const std::string& db) {
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(MakeRef("SearchPhrase"));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeMinAgg(MakeRef("URL")); });
        agg_factories.push_back([]() { return MakeCountAgg(); });
        return std::make_unique<OrderBy>(
            std::make_unique<GroupBy>(
                std::make_unique<Filter>(
                    std::make_unique<Scan>(
                        db, Schema({{"URL", Type::String}, {"SearchPhrase", Type::String}})),
                    And(Like(MakeRef("URL"), "%google%"),
                        Ne(MakeRef("SearchPhrase"), MakeConst(std::string(""))))),
                std::move(key_exprs), std::vector<std::string>{"SearchPhrase"},
                std::vector<std::string>{"MIN(URL)", "COUNT(*)"}, std::move(agg_factories)),
            std::vector<SortKey>{{"COUNT(*)", true}}, 10);
    },
    // Q22
    [](const std::string& db) {
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(MakeRef("SearchPhrase"));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeMinAgg(MakeRef("URL")); });
        agg_factories.push_back([]() { return MakeMinAgg(MakeRef("Title")); });
        agg_factories.push_back([]() { return MakeCountAgg(); });
        agg_factories.push_back([]() { return MakeCountDistinctAgg(MakeRef("UserID")); });
        return std::make_unique<OrderBy>(
            std::make_unique<GroupBy>(
                std::make_unique<Filter>(
                    std::make_unique<Scan>(db, Schema({{"URL", Type::String},
                                                       {"Title", Type::String},
                                                       {"SearchPhrase", Type::String},
                                                       {"UserID", Type::Int64}})),
                    And(And(Like(MakeRef("Title"), "%Google%"),
                            NotLike(MakeRef("URL"), "%.google.%")),
                        Ne(MakeRef("SearchPhrase"), MakeConst(std::string(""))))),
                std::move(key_exprs), std::vector<std::string>{"SearchPhrase"},
                std::vector<std::string>{"MIN(URL)", "MIN(Title)", "COUNT(*)",
                                         "COUNT(DISTINCT UserID)"},
                std::move(agg_factories)),
            std::vector<SortKey>{{"COUNT(*)", true}}, 10);
    },
    // Q23
    [](const std::string& db) {
        return std::make_unique<OrderBy>(
            std::make_unique<Filter>(std::make_unique<Scan>(db), Like(MakeRef("URL"), "%google%")),
            std::vector<SortKey>{{"EventTime", false}}, 10);
    },
    // Q24
    [](const std::string& db) {
        return std::make_unique<Project>(
            std::make_unique<OrderBy>(
                std::make_unique<Filter>(
                    std::make_unique<Scan>(db, Schema({{"SearchPhrase", Type::String},
                                                       {"EventTime", Type::Timestamp}})),
                    Ne(MakeRef("SearchPhrase"), MakeConst(std::string("")))),
                std::vector<SortKey>{{"EventTime", false}}, 10),
            std::vector<std::string>{"SearchPhrase"});
    },
    // Q25
    [](const std::string& db) {
        return std::make_unique<OrderBy>(
            std::make_unique<Filter>(
                std::make_unique<Scan>(db, Schema({{"SearchPhrase", Type::String}})),
                Ne(MakeRef("SearchPhrase"), MakeConst(std::string("")))),
            std::vector<SortKey>{{"SearchPhrase", false}}, 10);
    },
    // Q26
    [](const std::string& db) {
        return std::make_unique<Project>(
            std::make_unique<OrderBy>(
                std::make_unique<Filter>(
                    std::make_unique<Scan>(db, Schema({{"SearchPhrase", Type::String},
                                                       {"EventTime", Type::Timestamp}})),
                    Ne(MakeRef("SearchPhrase"), MakeConst(std::string("")))),
                std::vector<SortKey>{{"EventTime", false}, {"SearchPhrase", false}}, 10),
            std::vector<std::string>{"SearchPhrase"});
    },
    // Q27
    [](const std::string& db) {
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(MakeRef("CounterID"));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeAvgAgg(StringLength(MakeRef("URL"))); });
        agg_factories.push_back([]() { return MakeCountAgg(); });
        return std::make_unique<OrderBy>(
            std::make_unique<Filter>(
                std::make_unique<GroupBy>(
                    std::make_unique<Filter>(
                        std::make_unique<Scan>(
                            db, Schema({{"CounterID", Type::Int64}, {"URL", Type::String}})),
                        Ne(MakeRef("URL"), MakeConst(std::string("")))),
                    std::move(key_exprs), std::vector<std::string>{"CounterID"},
                    std::vector<std::string>{"l", "c"}, std::move(agg_factories)),
                Gt(MakeRef("c"), MakeConst(int64_t{100000}))),
            std::vector<SortKey>{{"l", true}}, 25);
    },
    // Q28
    [](const std::string& db) {
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(ExtractDomain(MakeRef("Referer")));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeAvgAgg(StringLength(MakeRef("Referer"))); });
        agg_factories.push_back([]() { return MakeCountAgg(); });
        agg_factories.push_back([]() { return MakeMinAgg(MakeRef("Referer")); });
        return std::make_unique<OrderBy>(
            std::make_unique<Filter>(
                std::make_unique<GroupBy>(
                    std::make_unique<Filter>(
                        std::make_unique<Scan>(db, Schema({{"Referer", Type::String}})),
                        Ne(MakeRef("Referer"), MakeConst(std::string("")))),
                    std::move(key_exprs), std::vector<std::string>{"k"},
                    std::vector<std::string>{"l", "c", "min_referer"}, std::move(agg_factories)),
                Gt(MakeRef("c"), MakeConst(int64_t{100000}))),
            std::vector<SortKey>{{"l", true}}, 25);
    },
    // Q29
    [](const std::string& db) {
        std::vector<std::string> names;
        std::vector<std::unique_ptr<Aggregator>> aggs;
        for (int i = 0; i <= 89; ++i) {
            names.push_back("sum" + std::to_string(i));
            aggs.push_back(MakeSumAgg(Plus(MakeRef("ResolutionWidth"), MakeConst(int64_t{i}))));
        }
        return std::make_unique<Aggregate>(
            std::make_unique<Scan>(db, Schema({{"ResolutionWidth", Type::Int64}})),
            std::move(names), std::move(aggs));
    },
    // Q30
    [](const std::string& db) {
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(MakeRef("SearchEngineID"));
        key_exprs.push_back(MakeRef("ClientIP"));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeCountAgg(); });
        agg_factories.push_back([]() { return MakeSumAgg(MakeRef("IsRefresh")); });
        agg_factories.push_back([]() { return MakeAvgAgg(MakeRef("ResolutionWidth")); });
        return std::make_unique<OrderBy>(
            std::make_unique<GroupBy>(
                std::make_unique<Filter>(
                    std::make_unique<Scan>(db, Schema({{"SearchEngineID", Type::Int64},
                                                       {"ClientIP", Type::Int64},
                                                       {"SearchPhrase", Type::String},
                                                       {"IsRefresh", Type::Int64},
                                                       {"ResolutionWidth", Type::Int64}})),
                    Ne(MakeRef("SearchPhrase"), MakeConst(std::string("")))),
                std::move(key_exprs), std::vector<std::string>{"SearchEngineID", "ClientIP"},
                std::vector<std::string>{"c", "sum_refresh", "avg_width"},
                std::move(agg_factories)),
            std::vector<SortKey>{{"c", true}}, 10);
    },
    // Q31
    [](const std::string& db) {
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(MakeRef("WatchID"));
        key_exprs.push_back(MakeRef("ClientIP"));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeCountAgg(); });
        agg_factories.push_back([]() { return MakeSumAgg(MakeRef("IsRefresh")); });
        agg_factories.push_back([]() { return MakeAvgAgg(MakeRef("ResolutionWidth")); });
        return std::make_unique<OrderBy>(
            std::make_unique<GroupBy>(
                std::make_unique<Filter>(
                    std::make_unique<Scan>(db, Schema({{"WatchID", Type::Int64},
                                                       {"ClientIP", Type::Int64},
                                                       {"SearchPhrase", Type::String},
                                                       {"IsRefresh", Type::Int64},
                                                       {"ResolutionWidth", Type::Int64}})),
                    Ne(MakeRef("SearchPhrase"), MakeConst(std::string("")))),
                std::move(key_exprs), std::vector<std::string>{"WatchID", "ClientIP"},
                std::vector<std::string>{"c", "sum_refresh", "avg_width"},
                std::move(agg_factories)),
            std::vector<SortKey>{{"c", true}}, 10);
    },
    // Q32
    [](const std::string& db) {
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(MakeRef("WatchID"));
        key_exprs.push_back(MakeRef("ClientIP"));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeCountAgg(); });
        agg_factories.push_back([]() { return MakeSumAgg(MakeRef("IsRefresh")); });
        agg_factories.push_back([]() { return MakeAvgAgg(MakeRef("ResolutionWidth")); });
        return std::make_unique<OrderBy>(
            std::make_unique<GroupBy>(
                std::make_unique<Scan>(db, Schema({{"WatchID", Type::Int64},
                                                   {"ClientIP", Type::Int64},
                                                   {"IsRefresh", Type::Int64},
                                                   {"ResolutionWidth", Type::Int64}})),
                std::move(key_exprs), std::vector<std::string>{"WatchID", "ClientIP"},
                std::vector<std::string>{"c", "sum_refresh", "avg_width"},
                std::move(agg_factories)),
            std::vector<SortKey>{{"c", true}}, 10);
    },
    // Q33
    [](const std::string& db) {
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(MakeRef("URL"));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeCountAgg(); });
        return std::make_unique<OrderBy>(
            std::make_unique<GroupBy>(std::make_unique<Scan>(db, Schema({{"URL", Type::String}})),
                                      std::move(key_exprs), std::vector<std::string>{"URL"},
                                      std::vector<std::string>{"c"}, std::move(agg_factories)),
            std::vector<SortKey>{{"c", true}}, 10);
    },
    // Q34
    [](const std::string& db) {
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(MakeConst(int64_t{1}));
        key_exprs.push_back(MakeRef("URL"));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeCountAgg(); });
        return std::make_unique<OrderBy>(
            std::make_unique<GroupBy>(std::make_unique<Scan>(db, Schema({{"URL", Type::String}})),
                                      std::move(key_exprs), std::vector<std::string>{"1", "URL"},
                                      std::vector<std::string>{"c"}, std::move(agg_factories)),
            std::vector<SortKey>{{"c", true}}, 10);
    },
    // Q35
    [](const std::string& db) {
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(MakeRef("ClientIP"));
        key_exprs.push_back(Plus(MakeRef("ClientIP"), MakeConst(int64_t{-1})));
        key_exprs.push_back(Plus(MakeRef("ClientIP"), MakeConst(int64_t{-2})));
        key_exprs.push_back(Plus(MakeRef("ClientIP"), MakeConst(int64_t{-3})));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeCountAgg(); });
        return std::make_unique<OrderBy>(
            std::make_unique<GroupBy>(
                std::make_unique<Scan>(db, Schema({{"ClientIP", Type::Int64}})),
                std::move(key_exprs),
                std::vector<std::string>{"ClientIP", "ClientIP-1", "ClientIP-2", "ClientIP-3"},
                std::vector<std::string>{"c"}, std::move(agg_factories)),
            std::vector<SortKey>{{"c", true}}, 10);
    },
    // Q36
    [](const std::string& db) {
        auto filter = And(Eq(MakeRef("CounterID"), MakeConst(int64_t{62})),
                          And(Ge(MakeRef("EventDate"), MakeConst(std::string("2013-07-01"))),
                              And(Le(MakeRef("EventDate"), MakeConst(std::string("2013-07-31"))),
                                  And(Eq(MakeRef("DontCountHits"), MakeConst(int64_t{0})),
                                      And(Eq(MakeRef("IsRefresh"), MakeConst(int64_t{0})),
                                          Ne(MakeRef("URL"), MakeConst(std::string(""))))))));
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(MakeRef("URL"));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeCountAgg(); });
        return std::make_unique<OrderBy>(
            std::make_unique<GroupBy>(
                std::make_unique<Filter>(
                    std::make_unique<Scan>(db, Schema({{"CounterID", Type::Int64},
                                                       {"EventDate", Type::Date},
                                                       {"DontCountHits", Type::Int64},
                                                       {"IsRefresh", Type::Int64},
                                                       {"URL", Type::String}})),
                    std::move(filter)),
                std::move(key_exprs), std::vector<std::string>{"URL"},
                std::vector<std::string>{"PageViews"}, std::move(agg_factories)),
            std::vector<SortKey>{{"PageViews", true}}, 10);
    },
    // Q37
    [](const std::string& db) {
        auto filter = And(Eq(MakeRef("CounterID"), MakeConst(int64_t{62})),
                          And(Ge(MakeRef("EventDate"), MakeConst(std::string("2013-07-01"))),
                              And(Le(MakeRef("EventDate"), MakeConst(std::string("2013-07-31"))),
                                  And(Eq(MakeRef("DontCountHits"), MakeConst(int64_t{0})),
                                      And(Eq(MakeRef("IsRefresh"), MakeConst(int64_t{0})),
                                          Ne(MakeRef("Title"), MakeConst(std::string(""))))))));
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(MakeRef("Title"));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeCountAgg(); });
        return std::make_unique<OrderBy>(
            std::make_unique<GroupBy>(
                std::make_unique<Filter>(
                    std::make_unique<Scan>(db, Schema({{"CounterID", Type::Int64},
                                                       {"Title", Type::String},
                                                       {"EventDate", Type::Date},
                                                       {"DontCountHits", Type::Int64},
                                                       {"IsRefresh", Type::Int64}})),
                    std::move(filter)),
                std::move(key_exprs), std::vector<std::string>{"Title"},
                std::vector<std::string>{"PageViews"}, std::move(agg_factories)),
            std::vector<SortKey>{{"PageViews", true}}, 10);
    },
    // Q38
    [](const std::string& db) {
        auto filter = And(Eq(MakeRef("CounterID"), MakeConst(int64_t{62})),
                          And(Ge(MakeRef("EventDate"), MakeConst(std::string("2013-07-01"))),
                              And(Le(MakeRef("EventDate"), MakeConst(std::string("2013-07-31"))),
                                  And(Eq(MakeRef("IsRefresh"), MakeConst(int64_t{0})),
                                      And(Ne(MakeRef("IsLink"), MakeConst(int64_t{0})),
                                          Eq(MakeRef("IsDownload"), MakeConst(int64_t{0})))))));
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(MakeRef("URL"));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeCountAgg(); });
        return std::make_unique<OrderBy>(
            std::make_unique<GroupBy>(
                std::make_unique<Filter>(
                    std::make_unique<Scan>(db, Schema({{"CounterID", Type::Int64},
                                                       {"EventDate", Type::Date},
                                                       {"IsRefresh", Type::Int64},
                                                       {"IsLink", Type::Int64},
                                                       {"IsDownload", Type::Int64},
                                                       {"URL", Type::String}})),
                    std::move(filter)),
                std::move(key_exprs), std::vector<std::string>{"URL"},
                std::vector<std::string>{"PageViews"}, std::move(agg_factories)),
            std::vector<SortKey>{{"PageViews", true}}, 10, 1000);
    },
    // Q39
    [](const std::string& db) {
        auto filter = And(Eq(MakeRef("CounterID"), MakeConst(int64_t{62})),
                          And(Ge(MakeRef("EventDate"), MakeConst(std::string("2013-07-01"))),
                              And(Le(MakeRef("EventDate"), MakeConst(std::string("2013-07-31"))),
                                  Eq(MakeRef("IsRefresh"), MakeConst(int64_t{0})))));
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(MakeRef("TraficSourceID"));
        key_exprs.push_back(MakeRef("SearchEngineID"));
        key_exprs.push_back(MakeRef("AdvEngineID"));
        key_exprs.push_back(CaseWhen(And(Eq(MakeRef("SearchEngineID"), MakeConst(int64_t{0})),
                                         Eq(MakeRef("AdvEngineID"), MakeConst(int64_t{0}))),
                                     MakeRef("Referer"), MakeConst(std::string(""))));
        key_exprs.push_back(MakeRef("URL"));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeCountAgg(); });
        return std::make_unique<OrderBy>(
            std::make_unique<GroupBy>(
                std::make_unique<Filter>(
                    std::make_unique<Scan>(db, Schema({{"CounterID", Type::Int64},
                                                       {"EventDate", Type::Date},
                                                       {"IsRefresh", Type::Int64},
                                                       {"TraficSourceID", Type::Int64},
                                                       {"SearchEngineID", Type::Int64},
                                                       {"AdvEngineID", Type::Int64},
                                                       {"Referer", Type::String},
                                                       {"URL", Type::String}})),
                    std::move(filter)),
                std::move(key_exprs),
                std::vector<std::string>{"TraficSourceID", "SearchEngineID", "AdvEngineID", "Src",
                                         "Dst"},
                std::vector<std::string>{"PageViews"}, std::move(agg_factories)),
            std::vector<SortKey>{{"PageViews", true}}, 10, 1000);
    },
    // Q40
    [](const std::string& db) {
        auto filter = And(Eq(MakeRef("CounterID"), MakeConst(int64_t{62})),
                          And(Ge(MakeRef("EventDate"), MakeConst(std::string("2013-07-01"))),
                              And(Le(MakeRef("EventDate"), MakeConst(std::string("2013-07-31"))),
                                  And(Eq(MakeRef("IsRefresh"), MakeConst(int64_t{0})),
                                      And(Or(Eq(MakeRef("TraficSourceID"), MakeConst(int64_t{-1})),
                                             Eq(MakeRef("TraficSourceID"), MakeConst(int64_t{6}))),
                                          Eq(MakeRef("RefererHash"),
                                             MakeConst(int64_t{3594120000172545465LL})))))));
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(MakeRef("URLHash"));
        key_exprs.push_back(MakeRef("EventDate"));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeCountAgg(); });
        return std::make_unique<OrderBy>(
            std::make_unique<GroupBy>(
                std::make_unique<Filter>(
                    std::make_unique<Scan>(db, Schema({{"CounterID", Type::Int64},
                                                       {"EventDate", Type::Date},
                                                       {"IsRefresh", Type::Int64},
                                                       {"TraficSourceID", Type::Int64},
                                                       {"RefererHash", Type::Int64},
                                                       {"URLHash", Type::Int64}})),
                    std::move(filter)),
                std::move(key_exprs), std::vector<std::string>{"URLHash", "EventDate"},
                std::vector<std::string>{"PageViews"}, std::move(agg_factories)),
            std::vector<SortKey>{{"PageViews", true}}, 10, 100);
    },
    // Q41
    [](const std::string& db) {
        auto filter = And(
            Eq(MakeRef("CounterID"), MakeConst(int64_t{62})),
            And(Ge(MakeRef("EventDate"), MakeConst(std::string("2013-07-01"))),
                And(Le(MakeRef("EventDate"), MakeConst(std::string("2013-07-31"))),
                    And(Eq(MakeRef("IsRefresh"), MakeConst(int64_t{0})),
                        And(Eq(MakeRef("DontCountHits"), MakeConst(int64_t{0})),
                            Eq(MakeRef("URLHash"), MakeConst(int64_t{2868770270353813622LL})))))));
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(MakeRef("WindowClientWidth"));
        key_exprs.push_back(MakeRef("WindowClientHeight"));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeCountAgg(); });
        return std::make_unique<OrderBy>(
            std::make_unique<GroupBy>(
                std::make_unique<Filter>(
                    std::make_unique<Scan>(db, Schema({{"CounterID", Type::Int64},
                                                       {"EventDate", Type::Date},
                                                       {"IsRefresh", Type::Int64},
                                                       {"DontCountHits", Type::Int64},
                                                       {"URLHash", Type::Int64},
                                                       {"WindowClientWidth", Type::Int64},
                                                       {"WindowClientHeight", Type::Int64}})),
                    std::move(filter)),
                std::move(key_exprs),
                std::vector<std::string>{"WindowClientWidth", "WindowClientHeight"},
                std::vector<std::string>{"PageViews"}, std::move(agg_factories)),
            std::vector<SortKey>{{"PageViews", true}}, 10, 10000);
    },
    // Q42
    [](const std::string& db) {
        auto filter = And(Eq(MakeRef("CounterID"), MakeConst(int64_t{62})),
                          And(Ge(MakeRef("EventDate"), MakeConst(std::string("2013-07-14"))),
                              And(Le(MakeRef("EventDate"), MakeConst(std::string("2013-07-15"))),
                                  And(Eq(MakeRef("IsRefresh"), MakeConst(int64_t{0})),
                                      Eq(MakeRef("DontCountHits"), MakeConst(int64_t{0}))))));
        std::vector<std::unique_ptr<Expression>> key_exprs;
        key_exprs.push_back(DateTruncMinute(MakeRef("EventTime")));
        std::vector<AggFactory> agg_factories;
        agg_factories.push_back([]() { return MakeCountAgg(); });
        return std::make_unique<OrderBy>(
            std::make_unique<GroupBy>(
                std::make_unique<Filter>(
                    std::make_unique<Scan>(db, Schema({{"CounterID", Type::Int64},
                                                       {"EventDate", Type::Date},
                                                       {"EventTime", Type::Timestamp},
                                                       {"IsRefresh", Type::Int64},
                                                       {"DontCountHits", Type::Int64}})),
                    std::move(filter)),
                std::move(key_exprs), std::vector<std::string>{"M"},
                std::vector<std::string>{"PageViews"}, std::move(agg_factories)),
            std::vector<SortKey>{{"M", false}}, 10, 1000);
    },
};

static void PrintCsvField(const std::string& s) {
    if (s.find_first_of(",\"\n") == std::string::npos) {
        std::cout << s;
        return;
    }
    std::cout << '"';
    for (char c : s) {
        if (c == '"') {
            std::cout << '"';
        }
        std::cout << c;
    }
    std::cout << '"';
}

static void PrintBlock(const Block& block) {
    for (size_t row = 0; row < block.row_count; ++row) {
        for (size_t col = 0; col < block.names.size(); ++col) {
            if (col != 0) {
                std::cout << ',';
            }
            PrintCsvField(ToString(block.columns[col]->Get(row)));
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
