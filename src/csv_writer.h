#pragma once

#include <vector>
#include <fstream>
#include "csv.h"

class CsvWriter {
public:
    CsvWriter(const std::string& filename, CsvConfig config = {});

    void WriteRow(const std::vector<std::string>&);

private:
    std::ofstream output_;
    CsvConfig config_{};
};