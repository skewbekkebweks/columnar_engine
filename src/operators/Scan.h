#pragma once

#include <optional>
#include <string>
#include <vector>
#include "columnar_reader.h"
#include "operators/operator.h"
#include "schema.h"

class Scan : public Operator {
public:
    Scan(std::string filename, Schema projection);
    std::optional<Block> Next() override;

private:
    ColumnarReader reader_;
    std::vector<size_t> col_indices_;
    std::vector<std::string> col_names_;
};
