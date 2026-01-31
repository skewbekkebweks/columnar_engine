#pragma once

#include <fstream>
#include <memory>
#include "metadata.h"
#include "column.h"

class ColumnarWriter {
public:
    ColumnarWriter(const std::string& filename, const Schema& schema);

    void AddRowGroup(const std::vector<std::unique_ptr<Column>>& columns);

    void Finalize();

private:
    std::ofstream output_;
    Metadata metadata_;

    bool is_finilized_ = false;
};