#pragma once

#include <unordered_set>
#include <vector>
#include <string>
#include "type.h"
#include "csv.h"

class Schema {
public:
    struct ColumnInfo {
        std::string name;
        Type type;
    };

    Schema();
    Schema(std::vector<Schema::ColumnInfo>);

    void AddColumn(Schema::ColumnInfo);

    static Schema FromCsv(const std::string& filename, CsvConfig config = {});

    size_t Size() const;
    const std::vector<Schema::ColumnInfo>& GetColumns() const;

private:
    std::vector<Schema::ColumnInfo> columns_;
    std::unordered_set<std::string> column_names_;
};