#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include "columnar_reader.h"
#include "operators/operator.h"
#include "schema.h"

class Scan : public Operator {
public:
    explicit Scan(std::string filename);
    Scan(std::string filename, Schema projection);

    void AddRangeHint(const std::string& column, int64_t begin, int64_t end);

    std::optional<Block> Next() override;

private:
    struct RangeHint {
        size_t col_idx;
        int64_t begin;
        int64_t end;
    };

    bool CanSkipCurrentRowGroup() const;

    ColumnarReader reader_;
    std::vector<size_t> col_indices_;
    std::vector<std::string> col_names_;
    std::vector<RangeHint> range_hints_;
};
