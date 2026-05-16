#include "metadata.h"

Metadata::Metadata() : schema_() {
}

Metadata::Metadata(const Schema& schema) : schema_(schema) {
}

void Metadata::SetSchema(const Schema& schema) {
    schema_ = schema;
}

void Metadata::AddRowGroup(size_t offset, size_t row_count, std::vector<size_t> col_offsets) {
    offsets_.push_back(offset);
    row_counts_.push_back(row_count);
    col_offsets_.push_back(std::move(col_offsets));
}

size_t Metadata::GetColumnsCount() const {
    return schema_.Size();
}
size_t Metadata::GetRowGroupsCount() const {
    return offsets_.size();
}
const Schema& Metadata::GetSchema() const {
    return schema_;
}
const std::vector<size_t>& Metadata::GetOffsets() const {
    return offsets_;
}
const std::vector<size_t>& Metadata::GetRowCounts() const {
    return row_counts_;
}

const std::vector<std::vector<size_t>>& Metadata::GetColOffsets() const {
    return col_offsets_;
}