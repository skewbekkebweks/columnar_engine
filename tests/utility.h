#include <string>
#include <fstream>
#include <memory>
#include <vector>
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