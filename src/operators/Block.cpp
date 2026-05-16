#include "operators/Block.h"

#include "error.h"

void Block::AddColumn(std::string name, std::unique_ptr<Column> col) {
    names.push_back(std::move(name));
    columns.push_back(std::move(col));
}

const Column& Block::GetColumn(const std::string& name) const {
    for (size_t i = 0; i < names.size(); ++i) {
        if (names[i] == name) {
            return *columns[i];
        }
    }
    THROW_RUNTIME_ERROR("column not found: " + name);
}

bool Block::HasColumn(const std::string& name) const {
    for (const auto& n : names) {
        if (n == name) {
            return true;
        }
    }
    return false;
}
