#include "operators/project.h"

Project::Project(std::unique_ptr<Operator> child, std::vector<std::string> columns)
    : child_(std::move(child)), columns_(std::move(columns)) {
}

std::optional<Block> Project::Next() {
    auto block = child_->Next();
    if (!block) {
        return std::nullopt;
    }

    Block result;
    result.row_count = block->row_count;
    for (const auto& name : columns_) {
        for (size_t c = 0; c < block->names.size(); ++c) {
            if (block->names[c] == name) {
                result.AddColumn(name, std::move(block->columns[c]));
                break;
            }
        }
    }
    return result;
}
