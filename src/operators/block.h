#pragma once

#include <memory>
#include <string>
#include <vector>
#include "column.h"

struct Block {
    std::vector<std::string> names;
    std::vector<std::unique_ptr<Column>> columns;
    size_t row_count = 0;

    void AddColumn(std::string name, std::unique_ptr<Column> col);
    const Column& GetColumn(const std::string& name) const;
    bool HasColumn(const std::string& name) const;
};
