#pragma once

#include "schema.h"

class Metadata {
public:
    Metadata();
    Metadata(const Schema& schema);

    void SetSchema(const Schema& schema);

    void AddRowGroup(size_t offset, size_t row_count);

    size_t GetColumnsCount() const;
    size_t GetRowGroupsCount() const;
    Schema GetSchema() const;
    std::vector<size_t> GetOffsets() const;
    std::vector<size_t> GetRowCounts() const;
private:
    Schema schema_;
    std::vector<size_t> offsets_;
    std::vector<size_t> row_counts_;
};