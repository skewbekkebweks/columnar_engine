#include "operators/scan.h"

#include <algorithm>
#include "error.h"

Scan::Scan(std::string filename, Schema projection) : reader_(std::move(filename)) {
    const auto& proj_cols = projection.GetColumns();
    if (proj_cols.empty()) {
        return;
    }

    const auto& file_cols = reader_.GetSchema().GetColumns();

    std::vector<std::pair<size_t, std::string>> mapping;
    mapping.reserve(proj_cols.size());

    for (const auto& pc : proj_cols) {
        bool found = false;
        for (size_t i = 0; i < file_cols.size(); ++i) {
            if (file_cols[i].name == pc.name) {
                mapping.emplace_back(i, pc.name);
                found = true;
                break;
            }
        }
        if (!found) {
            THROW_RUNTIME_ERROR("Scan: column not found in file: " + pc.name);
        }
    }

    std::sort(mapping.begin(), mapping.end());

    for (auto& [idx, name] : mapping) {
        col_indices_.push_back(idx);
        col_names_.push_back(std::move(name));
    }
}

std::optional<Block> Scan::Next() {
    if (!reader_.HasRowGroup()) {
        return std::nullopt;
    }

    if (col_indices_.empty()) {
        Block result;
        result.row_count = reader_.SkipRowGroup();
        return result;
    }

    auto rg_opt = reader_.ReadRowGroup(&col_indices_);
    if (!rg_opt) {
        return std::nullopt;
    }
    auto& rg = *rg_opt;

    Block result;
    result.row_count = rg[0]->Size();
    for (size_t j = 0; j < col_indices_.size(); ++j) {
        result.AddColumn(col_names_[j], std::move(rg[j]));
    }
    return result;
}
