#pragma once

#include <string>
#include <fstream>
#include <optional>

#include "metadata.h"
#include "column.h"

class ColumnarReader {
public:
    ColumnarReader(const std::string& filename);

    bool HasRowGroup();

    std::optional<std::vector<std::unique_ptr<Column>>> ReadRowGroup();

private:
    void ScanMetadata();
    Schema ReadSchema(size_t columns_count);

    std::ifstream input_;
    Metadata metadata_;

    size_t cur_row_group_idx{0};
};