#include "columnar_writer.h"

#include "spdlog/spdlog.h"

#include "error.h"
#include "file_writer.h"

ColumnarWriter::ColumnarWriter(const std::string& filename, const Schema& schema)
    : output_(filename), metadata_(schema) {
}

void ColumnarWriter::AddRowGroup(const std::vector<std::unique_ptr<Column>>& columns) {
    if (columns.size() != metadata_.GetColumnsCount()) {
        THROW_RUNTIME_ERROR("actual columns count is different from metadata's columns count");
    }

    size_t expected_size = columns[0]->Size();
    size_t rg_offset = output_.tellp();

    std::vector<size_t> col_offsets;
    col_offsets.reserve(columns.size());

    for (size_t i = 0; i < columns.size(); ++i) {
        spdlog::debug("ColumnarWriter::AddRowGroup: {}", i);
        if (columns[i]->Size() != expected_size) {
            THROW_RUNTIME_ERROR("columns[" + std::to_string(i) +
                                "] and columns[0] have different size");
        }
        col_offsets.push_back(output_.tellp());
        columns[i]->Write(output_);
    }

    metadata_.AddRowGroup(rg_offset, expected_size, std::move(col_offsets));
}

void ColumnarWriter::Finalize() && {
    Schema schema = metadata_.GetSchema();
    const auto& offsets = metadata_.GetOffsets();
    const auto& row_counts = metadata_.GetRowCounts();
    const auto& col_offsets = metadata_.GetColOffsets();

    spdlog::debug("ColumnarWriter::Finalize " + std::to_string(output_.tellp()) + " " +
                  std::to_string(offsets.size()));

    for (size_t i = 0; i < offsets.size(); ++i) {
        Write(output_, offsets[i]);
        Write(output_, row_counts[i]);
        for (size_t col_offset : col_offsets[i]) {
            Write(output_, col_offset);
        }
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