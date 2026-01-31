#include "csv_reader.h"

CsvReader::CsvReader(const std::string& filename, CsvConfig config) : input_(filename), config_(config) {}

struct CsvReader::CsvRowState {
    std::vector<std::string> row = {""};
    bool in_quotes = false;
};

bool CsvReader::UpdateState(CsvRowState& state, char c) {
    if (c == EOF) {
        return false;
    }
    if (c == config_.quote) {
        if (state.in_quotes) {
            if (input_.peek() == config_.quote) {
                state.row.back() += config_.quote;
            } else {
                state.in_quotes = false;
            }
        } else {
            state.in_quotes = true;
        }
    } else if (state.in_quotes) {
        state.row.back() += c;
    } else if (c == config_.delimiter) {
        state.row.push_back("");
    } else if (c == '\n') {
        return false;
    } else {
        state.row.back() += c;
    }
    return true;
}

std::optional<std::vector<std::string>> CsvReader::ReadRow() {
    if (!input_ || input_.eof()) {
        return std::nullopt;
    }
    CsvRowState state;

    while (true) {
        char c = input_.get();
        bool read_next = UpdateState(state, c);
        if (!read_next) {
            break;
        }
    }

    if (state.row.size() == 1 && state.row[0].empty()) {
        return std::nullopt;
    }

    return state.row;
}