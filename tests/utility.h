#include <string>
#include <fstream>
#include <memory>
#include <vector>
#include "operators/block.h"
#include "column.h"
#include "schema.h"
#include "columnar_writer.h"

inline std::string CreateTempFile(const std::string& content, const std::string& filename = "a.txt") {
    std::string path = "/tmp/" + filename;
    std::ofstream out(path);
    out << content;
    out.close();
    return path;
}

inline std::string MakeTempDb(const std::string& filename, const Schema& schema,
                               const std::vector<std::vector<std::string>>& rows) {
    std::string path = "/tmp/" + filename;
    ColumnarWriter writer(path, schema);

    const auto& cols_info = schema.GetColumns();
    std::vector<std::unique_ptr<Column>> columns;
    for (const auto& column : cols_info) {
        columns.push_back(MakeColumn(column.type));
    }
    for (const auto& row : rows) {
        for (size_t i = 0; i < row.size(); ++i) {
            columns[i]->PushBack(row[i]);
        }
    }
    writer.AddRowGroup(columns);
    std::move(writer).Finalize();
    return path;
}

inline std::vector<std::unique_ptr<Column>> MakeCols(Schema& schema,
                                                      std::vector<std::vector<std::string>> data) {
    std::vector<std::unique_ptr<Column>> cols;
    for (size_t c = 0; c < schema.GetColumns().size(); ++c) {
        auto col = MakeColumn(schema.GetColumns()[c].type);
        for (auto& row : data) {
            col->PushBack(row[c]);
        }
        cols.push_back(std::move(col));
    }
    return cols;
}

inline std::vector<std::vector<Value>> Rows(const Block& b) {
    std::vector<std::vector<Value>> rows;
    for (size_t r = 0; r < b.row_count; ++r) {
        std::vector<Value> row;
        for (size_t c = 0; c < b.columns.size(); ++c) {
            row.push_back(b.columns[c]->Get(r));
        }
        rows.push_back(std::move(row));
    }
    return rows;
}

inline void SortRows(std::vector<std::vector<Value>>& rows) {
    std::sort(rows.begin(), rows.end(), [](const auto& a, const auto& b) {
        return Less(a[0], b[0]);
    });
}

inline Block OneRow(
    std::vector<std::pair<std::string, int64_t>> ints = {},
    std::vector<std::pair<std::string, std::string>> strs = {})
{
    Block b;
    b.row_count = 1;
    for (auto& [name, val] : ints) {
        auto col = std::make_unique<ColumnInt64>();
        col->PushBack(Value{val});
        b.AddColumn(name, std::move(col));
    }
    for (auto& [name, val] : strs) {
        auto col = std::make_unique<ColumnString>();
        col->PushBack(Value{val});
        b.AddColumn(name, std::move(col));
    }
    return b;
}