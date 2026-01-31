#pragma once

#include <vector>
#include <fstream>
#include <optional>
#include "csv.h"

class CsvReader {
public:
    CsvReader(const std::string& filename, CsvConfig config = {});
    
    std::optional<std::vector<std::string>> ReadRow();

private:
    struct CsvRowState;
    bool UpdateState(CsvRowState& state, char c);

    std::ifstream input_;
    CsvConfig config_{};
};