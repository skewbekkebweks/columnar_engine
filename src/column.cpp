#include "column.h"

#include <cstring>

#include <spdlog/spdlog.h>

#include "compress.h"
#include "error.h"

namespace {

template <typename T>
void AppendRows(std::vector<T>& dst, const std::vector<T>& src, const std::vector<size_t>& rows) {
    dst.reserve(dst.size() + rows.size());
    for (size_t r : rows) {
        dst.push_back(src[r]);
    }
}

template <typename Container>
void LoadStrings(Container& data, const char* raw, size_t row_count) {
    data.reserve(data.size() + row_count);
    const char* p = raw;
    for (size_t i = 0; i < row_count; ++i) {
        data.emplace_back(p);
        p += data.back().size() + 1;
    }
}

}

std::unique_ptr<Column> MakeColumn(Type type) {
    switch (type) {
        case Type::Int64:
            return std::make_unique<ColumnInt64>();
        case Type::Int128:
            return std::make_unique<ColumnInt128>();
        case Type::String:
            return std::make_unique<ColumnString>();
        case Type::Timestamp:
            return std::make_unique<ColumnTimestamp>();
        case Type::Date:
            return std::make_unique<ColumnDate>();
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

void ColumnInt64::AppendSelected(const Column& src, const std::vector<size_t>& rows) {
    AppendRows(data_, static_cast<const ColumnInt64&>(src).data_, rows);
}

void ColumnInt64::Write(std::ofstream& output) const {
    size_t uncompressed_size = data_.size() * sizeof(int64_t);
    const char* raw = reinterpret_cast<const char*>(data_.data());
    auto compressed = CompressLz4(raw, uncompressed_size);
    ::Write(output, uncompressed_size);
    ::Write(output, compressed.size());
    output.write(compressed.data(), compressed.size());
}

void ColumnInt64::LoadRaw(const char* data, size_t size, size_t row_count) {
    if (size != row_count * sizeof(int64_t)) {
        THROW_RUNTIME_ERROR("LoadRaw: size mismatch for int64 column");
    }
    size_t old_size = data_.size();
    data_.resize(old_size + row_count);
    std::memcpy(data_.data() + old_size, data, size);
}

// Int128

Type ColumnInt128::GetType() const {
    return Type::Int128;
}

void ColumnInt128::PushBack(const std::string&) {
    THROW_NOT_IMPLEMENTED;
}

void ColumnInt128::PushBack(const Value& value) {
    data_.push_back(std::get<__int128>(value));
}

size_t ColumnInt128::Size() const {
    return data_.size();
}

std::string ColumnInt128::operator[](size_t idx) const {
    return ToString(Value{data_[idx]});
}

Value ColumnInt128::Get(size_t idx) const {
    return data_[idx];
}

void ColumnInt128::AppendSelected(const Column& src, const std::vector<size_t>& rows) {
    AppendRows(data_, static_cast<const ColumnInt128&>(src).data_, rows);
}

void ColumnInt128::Write(std::ofstream&) const {
    THROW_NOT_IMPLEMENTED;
}

void ColumnInt128::LoadRaw(const char*, size_t, size_t) {
    THROW_NOT_IMPLEMENTED;
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

void ColumnString::AppendSelected(const Column& src, const std::vector<size_t>& rows) {
    AppendRows(data_, static_cast<const ColumnString&>(src).data_, rows);
}

void ColumnString::LoadRaw(const char* data, size_t, size_t row_count) {
    LoadStrings(data_, data, row_count);
}

void ColumnString::Write(std::ofstream& output) const {
    std::vector<char> raw;
    for (const std::string& s : data_) {
        raw.insert(raw.end(), s.begin(), s.end());
        raw.push_back('\0');
    }
    auto compressed = CompressLz4(raw.data(), raw.size());
    ::Write(output, raw.size());
    ::Write(output, compressed.size());
    output.write(compressed.data(), compressed.size());
}

// Timestamp

Type ColumnTimestamp::GetType() const {
    return Type::Timestamp;
}

void ColumnTimestamp::PushBack(const std::string& value) {
    data_.push_back(value);
}

void ColumnTimestamp::PushBack(const Value& value) {
    data_.push_back(std::get<std::string>(value));
}

size_t ColumnTimestamp::Size() const {
    return data_.size();
}

std::string ColumnTimestamp::operator[](size_t idx) const {
    return data_[idx];
}

Value ColumnTimestamp::Get(size_t idx) const {
    return data_[idx];
}

void ColumnTimestamp::AppendSelected(const Column& src, const std::vector<size_t>& rows) {
    AppendRows(data_, static_cast<const ColumnTimestamp&>(src).data_, rows);
}

void ColumnTimestamp::LoadRaw(const char* data, size_t, size_t row_count) {
    LoadStrings(data_, data, row_count);
}

void ColumnTimestamp::Write(std::ofstream& output) const {
    std::vector<char> raw;
    for (const std::string& s : data_) {
        raw.insert(raw.end(), s.begin(), s.end());
        raw.push_back('\0');
    }
    auto compressed = CompressLz4(raw.data(), raw.size());
    ::Write(output, raw.size());
    ::Write(output, compressed.size());
    output.write(compressed.data(), compressed.size());
}

// Date

Type ColumnDate::GetType() const {
    return Type::Date;
}

void ColumnDate::PushBack(const std::string& value) {
    data_.push_back(value);
}

void ColumnDate::PushBack(const Value& value) {
    data_.push_back(std::get<std::string>(value));
}

size_t ColumnDate::Size() const {
    return data_.size();
}

std::string ColumnDate::operator[](size_t idx) const {
    return data_[idx];
}

Value ColumnDate::Get(size_t idx) const {
    return data_[idx];
}

void ColumnDate::AppendSelected(const Column& src, const std::vector<size_t>& rows) {
    AppendRows(data_, static_cast<const ColumnDate&>(src).data_, rows);
}

void ColumnDate::LoadRaw(const char* data, size_t, size_t row_count) {
    LoadStrings(data_, data, row_count);
}

void ColumnDate::Write(std::ofstream& output) const {
    std::vector<char> raw;
    for (const std::string& s : data_) {
        raw.insert(raw.end(), s.begin(), s.end());
        raw.push_back('\0');
    }
    auto compressed = CompressLz4(raw.data(), raw.size());
    ::Write(output, raw.size());
    ::Write(output, compressed.size());
    output.write(compressed.data(), compressed.size());
}