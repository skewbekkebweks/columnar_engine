#include "csv_writer.h"

CsvWriter::CsvWriter(const std::string& filename, CsvConfig config) : output_(filename), config_(config) {}

void CsvWriter::WriteRow(const std::vector<std::string>& row) {
    size_t n = row.size();
    for (size_t i = 0; i < n; ++i) {
        bool need_quotes = false;
        for (char c : row[i]) {
            if (c == config_.delimiter || c == config_.quote || c == '\n') {
                need_quotes = true;
                break;
            }
        }

        if (need_quotes) {
            output_ << config_.quote;
        }

        for (char c : row[i]) {
            if (c == config_.quote) {
                output_ << config_.quote << config_.quote;
            } else {
                output_ << c;
            }
        }

        if (need_quotes) {
            output_ << config_.quote;
        }

        if (i == n - 1) {
            output_ << '\n';
        } else {
            output_ << config_.delimiter;
        }
    }
}