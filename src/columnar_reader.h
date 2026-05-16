#pragma once

#include <string>
#include <fstream>
#include <optional>

#include "metadata.h"
#include "column.h"

class ColumnarReader {
public:
    ColumnarReader(const std::string& filename);

    bool HasRowGroup() const;

    size_t SkipRowGroup();
    std::optional<std::vector<std::unique_ptr<Column>>> ReadRowGroup(
        const std::vector<size_t>* col_indices = nullptr);

    const Schema& GetSchema() const;
    size_t GetTotalRowCount() const;

private:
    void ScanMetadata();
    Schema ReadSchema(size_t columns_count);

    std::ifstream input_;
    Metadata metadata_;

    size_t cur_row_group_idx{0};
};