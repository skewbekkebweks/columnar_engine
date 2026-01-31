#include "schema.h"

#include "csv_reader.h"
#include "type.h"
#include <unordered_set>

Schema Schema::FromCsv(const std::string& filename, CsvConfig config) {
    CsvReader reader{filename, config};

    Schema schema;

    while (auto row_opt = reader.ReadRow()) {
        std::vector<std::string> row = row_opt.value();
        if (row.size() != 2) {
            throw std::runtime_error{"Each row in schema.csv should have 2 columns"};
        }
        schema.AddColumn(Schema::ColumnInfo{row[0], StringToType(row[1])});
    }

    return schema;
}

size_t Schema::Size() const {
    return columns_.size();
}

const std::vector<Schema::ColumnInfo>& Schema::GetColumns() const {
    return columns_;
}

Schema::Schema() : columns_() {}
Schema::Schema(std::vector<Schema::ColumnInfo> columns) : columns_(std::move(columns)) {}

void Schema::AddColumn(Schema::ColumnInfo column) {
    if (column_names_.count(column.name)) {
        throw std::runtime_error{"Schema has column " + column.name + " twice"};
    }
    column_names_.insert(column.name);
    columns_.push_back(column);
}