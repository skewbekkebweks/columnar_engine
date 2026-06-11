#pragma once

#include "schema.h"

class Metadata {
public:
    Metadata();
    Metadata(const Schema& schema);

    void SetSchema(const Schema& schema);

    void AddRowGroup(size_t offset, size_t row_count, std::vector<size_t> col_offsets);

    size_t GetColumnsCount() const;
    size_t GetRowGroupsCount() const;
    const Schema& GetSchema() const;
    const std::vector<size_t>& GetOffsets() const;
    const std::vector<size_t>& GetRowCounts() const;
    const std::vector<std::vector<size_t>>& GetColOffsets() const;

private:
    Schema schema_;
    std::vector<size_t> offsets_;
    std::vector<size_t> row_counts_;
    std::vector<std::vector<size_t>> col_offsets_;
};