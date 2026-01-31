#include "metadata.h"

Metadata::Metadata() : schema_() {}

Metadata::Metadata(const Schema& schema) : schema_(schema) {}

void Metadata::SetSchema(const Schema& schema) {
    schema_ = schema;
}

void Metadata::AddRowGroup(size_t offset, size_t row_count) {
    offsets_.push_back(offset);
    row_counts_.push_back(row_count);
}

size_t Metadata::GetColumnsCount() const {
    return schema_.Size();
}
size_t Metadata::GetRowGroupsCount() const {
    return offsets_.size();
}
Schema Metadata::GetSchema() const {
    return schema_;
}
std::vector<size_t> Metadata::GetOffsets() const {
    return offsets_;
}
std::vector<size_t> Metadata::GetRowCounts() const {
    return row_counts_;
}