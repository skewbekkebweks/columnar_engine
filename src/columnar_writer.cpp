#include "columnar_writer.h"

#include "spdlog/spdlog.h"

#include "file_writer.h"

ColumnarWriter::ColumnarWriter(const std::string& filename, const Schema& schema)
    : output_(filename), metadata_(schema) {
}

void ColumnarWriter::AddRowGroup(const std::vector<std::unique_ptr<Column>>& columns) {
    if (columns.size() != metadata_.GetColumnsCount()) {
        throw std::runtime_error{"actual columns count is different from metadata's columns count"};
    }

    size_t expected_size = columns[0]->Size();
    metadata_.AddRowGroup(output_.tellp(), expected_size);

    for (size_t i = 0; i < columns.size(); ++i) {
        spdlog::debug("ColumnarWriter::AddRowGroup: " + std::to_string(i));
        if (columns[i]->Size() != expected_size) {
            throw std::runtime_error{"columns[" + std::to_string(i) +
                                     "] and columns[0] have different size"};
        }
        columns[i]->Write(output_);
    }
}

void ColumnarWriter::Finalize() {
    if (is_finalized_) {
        return;
    }
    is_finalized_ = true;

    Schema schema = metadata_.GetSchema();
    std::vector<size_t> offsets = metadata_.GetOffsets();
    std::vector<size_t> row_counts = metadata_.GetRowCounts();

    spdlog::debug("ColumnarWriter::Finalize " + std::to_string(output_.tellp()) + " " +
                  std::to_string(offsets.size()));

    for (size_t i = 0; i < offsets.size(); ++i) {
        Write(output_, offsets[i]);
        Write(output_, row_counts[i]);
    }

    size_t schema_offset = output_.tellp();

    std::vector<Schema::ColumnInfo> columns = schema.GetColumns();
    for (size_t i = 0; i < columns.size(); ++i) {
        Write(output_, columns[i].name);
        Write(output_, static_cast<size_t>(columns[i].type));
    }

    Write(output_, schema_offset);
    Write(output_, schema.Size());
    Write(output_, offsets.size());
}