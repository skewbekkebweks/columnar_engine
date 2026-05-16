#include "columnar_reader.h"

#include <numeric>
#include "error.h"
#include "file_reader.h"
#include <spdlog/spdlog.h>

ColumnarReader::ColumnarReader(const std::string& filename) : input_(filename) {
    ScanMetadata();
}

bool ColumnarReader::HasRowGroup() const {
    return cur_row_group_idx < metadata_.GetRowGroupsCount();
}

size_t ColumnarReader::SkipRowGroup() {
    if (!HasRowGroup()) {
        THROW_RUNTIME_ERROR("SkipRowGroup: no row groups remaining");
    }
    size_t row_count = metadata_.GetRowCounts()[cur_row_group_idx];
    cur_row_group_idx++;
    if (HasRowGroup()) {
        input_.seekg(static_cast<std::streamoff>(metadata_.GetOffsets()[cur_row_group_idx]),
                     std::ios::beg);
    }
    return row_count;
}

std::optional<std::vector<std::unique_ptr<Column>>> ColumnarReader::ReadRowGroup(
    const std::vector<size_t>* col_indices) {
    if (!HasRowGroup()) {
        return std::nullopt;
    }

    const auto& columns = metadata_.GetSchema().GetColumns();
    size_t row_count = metadata_.GetRowCounts()[cur_row_group_idx];

    std::vector<size_t> all_indices;
    if (col_indices == nullptr) {
        all_indices.resize(columns.size());
        std::iota(all_indices.begin(), all_indices.end(), 0);
        col_indices = &all_indices;
    }

    std::vector<std::unique_ptr<Column>> result;
    result.reserve(col_indices->size());

    size_t want = 0;
    for (size_t col_pos = 0; col_pos < columns.size(); ++col_pos) {
        bool read_this = want < col_indices->size() && (*col_indices)[want] == col_pos;

        switch (columns[col_pos].type) {
            case Type::Int64:
                if (read_this) {
                    auto col = std::make_unique<ColumnInt64>();
                    for (size_t i = 0; i < row_count; ++i) {
                        col->PushBack(Value{Read<int64_t>(input_)});
                    }
                    result.push_back(std::move(col));
                    ++want;
                } else {
                    input_.seekg(static_cast<std::streamoff>(row_count * sizeof(int64_t)),
                                 std::ios::cur);
                }
                break;
            case Type::String:
                if (read_this) {
                    auto col = std::make_unique<ColumnString>();
                    for (size_t i = 0; i < row_count; ++i) {
                        col->PushBack(Value{Read<std::string>(input_)});
                    }
                    result.push_back(std::move(col));
                    ++want;
                } else {
                    std::string discard;
                    for (size_t i = 0; i < row_count; ++i) {
                        std::getline(input_, discard, '\0');
                    }
                }
                break;
            default:
                THROW_NOT_IMPLEMENTED;
        }
    }

    cur_row_group_idx++;
    return result;
}

const Schema& ColumnarReader::GetSchema() const {
    return metadata_.GetSchema();
}

size_t ColumnarReader::GetTotalRowCount() const {
    size_t total = 0;
    for (size_t count : metadata_.GetRowCounts()) {
        total += count;
    }
    return total;
}

void ColumnarReader::ScanMetadata() {
    input_.seekg(-sizeof(size_t), std::ios::end);
    size_t row_groups_count = Read<size_t>(input_);

    spdlog::debug("Row groups count is " + std::to_string(row_groups_count));

    input_.seekg(-2 * sizeof(size_t), std::ios::end);
    size_t columns_count = Read<size_t>(input_);

    spdlog::debug("Columns count is " + std::to_string(columns_count));

    input_.seekg(-3 * sizeof(size_t), std::ios::end);
    size_t schema_offset = Read<size_t>(input_);

    input_.seekg(schema_offset, std::ios::beg);
    Schema schema = ReadSchema(columns_count);

    metadata_.SetSchema(schema);

    size_t metadata_offset = schema_offset - 2 * sizeof(size_t) * row_groups_count;
    input_.seekg(metadata_offset, std::ios::beg);

    for (size_t i = 0; i < row_groups_count; ++i) {
        size_t offset = Read<size_t>(input_);
        size_t row_count = Read<size_t>(input_);
        spdlog::debug("Rows count is {}", row_count);
        metadata_.AddRowGroup(offset, row_count);
    }

    input_.seekg(0, std::ios::beg);
}

Schema ColumnarReader::ReadSchema(size_t columns_count) {
    Schema schema;

    for (size_t i = 0; i < columns_count; ++i) {
        std::string name = Read<std::string>(input_);
        Type type = static_cast<Type>(Read<size_t>(input_));

        spdlog::debug("ReadSchema: column " + name + " " +
                      std::to_string(static_cast<size_t>(type)));

        schema.AddColumn(Schema::ColumnInfo{name, type});
    }

    return schema;
}