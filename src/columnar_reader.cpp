#include "columnar_reader.h"

#include "file_reader.h"
#include <spdlog/spdlog.h>

ColumnarReader::ColumnarReader(const std::string& filename) : input_(filename) {
    ScanMetadata();
}

bool ColumnarReader::HasRowGroup() {
    return cur_row_group_idx < metadata_.GetRowGroupsCount();
}

std::optional<std::vector<std::unique_ptr<Column>>> ColumnarReader::ReadRowGroup() {
    if (!HasRowGroup()) {
        return std::nullopt;
    }

    Schema schema = metadata_.GetSchema();
    std::vector<Schema::ColumnInfo> columns = schema.GetColumns();

    size_t row_count = metadata_.GetRowCounts()[cur_row_group_idx];

    std::vector<std::unique_ptr<Column>> cur_row_group;
    cur_row_group.reserve(schema.Size());

    for (auto column : columns) {
        switch (column.type) {
        case Type::Int64: {
            cur_row_group.push_back(std::make_unique<ColumnInt64>());
            for (size_t i = 0; i < row_count; ++i) {
                int64_t value = Read<int64_t>(input_);
                spdlog::debug("Read " + std::to_string(value));
                cur_row_group.back()->PushBack(std::to_string(value));
            }
            break;
        }
        case Type::String: {
            cur_row_group.push_back(std::make_unique<ColumnString>());
            for (size_t i = 0; i < row_count; ++i) {
                std::string value = Read<std::string>(input_);
                spdlog::debug("Read " + value);
                cur_row_group.back()->PushBack(value);
            }
            break;
        }
        default:
            spdlog::error("Unreachable code reached in CsvToColumnar");
            throw std::runtime_error{"Unreachable code reached in CsvToColumnar"};
        }
    }

    cur_row_group_idx++;
    return cur_row_group;
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
        spdlog::debug("Rows count is " + std::to_string(row_count));
        metadata_.AddRowGroup(offset, row_count);
    }

    input_.seekg(0, std::ios::beg);
}

Schema ColumnarReader::ReadSchema(size_t columns_count) {
    Schema schema;

    for (size_t i = 0; i < columns_count; ++i) {
        std::string name = Read<std::string>(input_);
        Type type = static_cast<Type>(Read<size_t>(input_));

        spdlog::debug("ReadSchema: column " + name + " " + std::to_string(static_cast<size_t>(type)));

        schema.AddColumn(Schema::ColumnInfo{name, type});
    }

    return schema;
}