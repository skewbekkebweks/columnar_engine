#include "csv_reader.h"

CsvReader::CsvReader(const std::string& filename, CsvConfig config)
    : input_(filename), config_(config) {
}

struct CsvReader::CsvRowState {
    std::vector<std::string> row = {""};
    bool in_quotes = false;
};

bool CsvReader::UpdateState(CsvRowState& state, std::streambuf* sb, int c) {
    if (c == std::char_traits<char>::eof()) {
        return false;
    }
    if (c == config_.quote) {
        if (state.in_quotes) {
            if (sb->sgetc() == config_.quote) {
                state.row.back() += config_.quote;
                sb->sbumpc();
            } else {
                state.in_quotes = false;
            }
        } else {
            state.in_quotes = true;
        }
    } else if (state.in_quotes) {
        state.row.back() += static_cast<char>(c);
    } else if (c == config_.delimiter) {
        state.row.push_back("");
    } else if (c == '\n') {
        return false;
    } else {
        state.row.back() += static_cast<char>(c);
    }
    return true;
}

std::optional<std::vector<std::string>> CsvReader::ReadRow() {
    if (!input_) {
        return std::nullopt;
    }
    CsvRowState state;
    std::streambuf* sb = input_.rdbuf();

    while (true) {
        int c = sb->sbumpc();
        bool read_next = UpdateState(state, sb, c);
        if (!read_next) {
            break;
        }
    }

    if (state.row.size() == 1 && state.row[0].empty()) {
        return std::nullopt;
    }

    return state.row;
}