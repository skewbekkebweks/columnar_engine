#include "column.h"

#include <spdlog/spdlog.h>

// Int64

Type ColumnInt64::GetType() const {
    return Type::Int64;
}

void ColumnInt64::PushBack(const std::string& value) {
    data_.push_back(std::stoll(value));
}

size_t ColumnInt64::Size() const {
    return data_.size();
}

std::string ColumnInt64::operator[](size_t idx) {
    return std::to_string(data_[idx]);
}

void ColumnInt64::Write(std::ofstream& output) {
    spdlog::debug("ColumnInt64::Write");
    for (int64_t value : data_) {
        spdlog::debug("ColumnInt64::Write " + std::to_string(value));
        ::Write(output, value);
    }
}

// String

Type ColumnString::GetType() const {
    return Type::String;
}

void ColumnString::PushBack(const std::string& value) {
    data_.push_back(value);
}

size_t ColumnString::Size() const {
    return data_.size();
}

std::string ColumnString::operator[](size_t idx) {
    return data_[idx];
}

void ColumnString::Write(std::ofstream& output) {
    for (std::string value : data_) {
        ::Write(output, value);
    }
}