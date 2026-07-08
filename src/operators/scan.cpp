#include "operators/scan.h"

#include <algorithm>

#include <spdlog/spdlog.h>

#include "error.h"

Scan::Scan(std::string filename) : reader_(filename) {
    const auto& file_cols = reader_.GetSchema().GetColumns();
    for (size_t i = 0; i < file_cols.size(); ++i) {
        col_indices_.push_back(i);
        col_names_.push_back(file_cols[i].name);
    }
}

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

void Scan::AddRangeHint(const std::string& column, int64_t begin, int64_t end) {
    const auto& file_cols = reader_.GetSchema().GetColumns();
    for (size_t i = 0; i < file_cols.size(); ++i) {
        if (file_cols[i].name == column) {
            range_hints_.push_back(RangeHint{i, begin, end});
            return;
        }
    }
    THROW_RUNTIME_ERROR("Scan::AddRangeHint: column not found in file: " + column);
}

bool Scan::CanSkipCurrentRowGroup() const {
    const auto& stats = reader_.CurrentColStats();
    for (const auto& hint : range_hints_) {
        const ColumnStat& s = stats[hint.col_idx];
        if (s.valid && (hint.end < s.min || hint.begin > s.max)) {
            return true;
        }
    }
    return false;
}

std::optional<Block> Scan::Next() {
    while (reader_.HasRowGroup()) {
        if (CanSkipCurrentRowGroup()) {
            spdlog::debug("Scan: zone map pruned a row group");
            reader_.SkipRowGroup();
            continue;
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
    return std::nullopt;
}
