#include "csv_writer.h"

CsvWriter::CsvWriter(const std::string& filename, CsvConfig config)
    : output_(filename), config_(config) {
}

void CsvWriter::WriteRow(const std::vector<std::string>& row) {
    buf_.clear();
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
            buf_ += config_.quote;
            for (char c : row[i]) {
                if (c == config_.quote) {
                    buf_ += config_.quote;
                }
                buf_ += c;
            }
            buf_ += config_.quote;
        } else {
            buf_ += row[i];
        }

        if (i == n - 1) {
            buf_ += '\n';
        } else {
            buf_ += config_.delimiter;
        }
    }
    output_.write(buf_.data(), static_cast<std::streamsize>(buf_.size()));
}