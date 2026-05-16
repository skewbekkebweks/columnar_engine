#include "column.h"

#include <spdlog/spdlog.h>
#include "error.h"

std::unique_ptr<Column> MakeColumn(Type type) {
    switch (type) {
        case Type::Int64:
            return std::make_unique<ColumnInt64>();
        case Type::String:
            return std::make_unique<ColumnString>();
        default:
            THROW_NOT_IMPLEMENTED;
    }
}

// Int64

Type ColumnInt64::GetType() const {
    return Type::Int64;
}

void ColumnInt64::PushBack(const std::string& value) {
    try {
        data_.push_back(std::stoll(value));
    } catch (...) {
        THROW_RUNTIME_ERROR("invalid int64");
    }
}

void ColumnInt64::PushBack(const Value& value) {
    data_.push_back(std::get<int64_t>(value));
}

size_t ColumnInt64::Size() const {
    return data_.size();
}

std::string ColumnInt64::operator[](size_t idx) const {
    return std::to_string(data_[idx]);
}

Value ColumnInt64::Get(size_t idx) const {
    return data_[idx];
}

void ColumnInt64::Write(std::ofstream& output) {
    for (int64_t value : data_) {
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

void ColumnString::PushBack(const Value& value) {
    data_.push_back(std::get<std::string>(value));
}

size_t ColumnString::Size() const {
    return data_.size();
}

std::string ColumnString::operator[](size_t idx) const {
    return data_[idx];
}

Value ColumnString::Get(size_t idx) const {
    return data_[idx];
}

void ColumnString::Write(std::ofstream& output) {
    for (const std::string& value : data_) {
        ::Write(output, value);
    }
}