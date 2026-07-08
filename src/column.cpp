#include "column.h"

#include "compress.h"
#include "date_util.h"
#include "error.h"

namespace {

template <typename T>
void AppendRows(std::vector<T>& dst, const std::vector<T>& src, const std::vector<size_t>& rows) {
    dst.reserve(dst.size() + rows.size());
    for (size_t r : rows) {
        dst.push_back(src[r]);
    }
}

void LoadStrings(std::vector<std::string>& data, const char* raw, size_t row_count) {
    data.reserve(data.size() + row_count);
    const char* p = raw;
    for (size_t i = 0; i < row_count; ++i) {
        data.emplace_back(p);
        p += data.back().size() + 1;
    }
}

template <typename T>
void LoadNumbers(std::vector<T>& data, const char* compressed, size_t compressed_size,
                 size_t uncompressed_size, size_t row_count) {
    size_t old_size = data.size();
    data.resize(old_size + row_count);
    DecompressLz4(compressed, compressed_size, reinterpret_cast<char*>(data.data() + old_size),
                  uncompressed_size);
}

void WriteCompressed(std::ofstream& output, const char* raw, size_t size) {
    auto compressed = CompressLz4(raw, size);
    Write(output, size);
    Write(output, compressed.size());
    output.write(compressed.data(), compressed.size());
}

template <typename T>
void WriteNumbers(std::ofstream& output, const std::vector<T>& data) {
    WriteCompressed(output, reinterpret_cast<const char*>(data.data()), data.size() * sizeof(T));
}

template <typename T>
bool MinMaxOf(const std::vector<T>& data, int64_t& min, int64_t& max) {
    if (data.empty()) {
        return false;
    }
    min = data[0];
    max = data[0];
    for (T v : data) {
        min = std::min<int64_t>(min, v);
        max = std::max<int64_t>(max, v);
    }
    return true;
}

}  // namespace

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
    WriteNumbers(output, data_);
}

void ColumnInt64::LoadCompressed(const char* compressed, size_t compressed_size,
                                 size_t uncompressed_size, size_t row_count) {
    LoadNumbers(data_, compressed, compressed_size, uncompressed_size, row_count);
}

const int64_t* ColumnInt64::AsInt64Data() const {
    return data_.data();
}

bool ColumnInt64::TryGetMinMax(int64_t& min, int64_t& max) const {
    return MinMaxOf(data_, min, max);
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

void ColumnInt128::LoadCompressed(const char*, size_t, size_t, size_t) {
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

void ColumnString::LoadCompressed(const char* compressed, size_t compressed_size,
                                  size_t uncompressed_size, size_t row_count) {
    auto raw = DecompressLz4(compressed, compressed_size, uncompressed_size);
    LoadStrings(data_, raw.data(), row_count);
}

void ColumnString::Write(std::ofstream& output) const {
    std::vector<char> raw;
    for (const std::string& s : data_) {
        raw.insert(raw.end(), s.begin(), s.end());
        raw.push_back('\0');
    }
    WriteCompressed(output, raw.data(), raw.size());
}

// Timestamp

Type ColumnTimestamp::GetType() const {
    return Type::Timestamp;
}

void ColumnTimestamp::PushBack(const std::string& value) {
    data_.push_back(ParseTimestamp(value));
}

void ColumnTimestamp::PushBack(const Value& value) {
    data_.push_back(std::get<int64_t>(value));
}

size_t ColumnTimestamp::Size() const {
    return data_.size();
}

std::string ColumnTimestamp::operator[](size_t idx) const {
    return FormatTimestamp(data_[idx]);
}

Value ColumnTimestamp::Get(size_t idx) const {
    return data_[idx];
}

void ColumnTimestamp::AppendSelected(const Column& src, const std::vector<size_t>& rows) {
    AppendRows(data_, static_cast<const ColumnTimestamp&>(src).data_, rows);
}

void ColumnTimestamp::LoadCompressed(const char* compressed, size_t compressed_size,
                                     size_t uncompressed_size, size_t row_count) {
    LoadNumbers(data_, compressed, compressed_size, uncompressed_size, row_count);
}

bool ColumnTimestamp::TryGetMinMax(int64_t& min, int64_t& max) const {
    return MinMaxOf(data_, min, max);
}

void ColumnTimestamp::Write(std::ofstream& output) const {
    WriteNumbers(output, data_);
}

// Date

Type ColumnDate::GetType() const {
    return Type::Date;
}

void ColumnDate::PushBack(const std::string& value) {
    data_.push_back(static_cast<int32_t>(ParseDate(value)));
}

void ColumnDate::PushBack(const Value& value) {
    data_.push_back(static_cast<int32_t>(std::get<int64_t>(value)));
}

size_t ColumnDate::Size() const {
    return data_.size();
}

std::string ColumnDate::operator[](size_t idx) const {
    return FormatDate(data_[idx]);
}

Value ColumnDate::Get(size_t idx) const {
    return static_cast<int64_t>(data_[idx]);
}

void ColumnDate::AppendSelected(const Column& src, const std::vector<size_t>& rows) {
    AppendRows(data_, static_cast<const ColumnDate&>(src).data_, rows);
}

void ColumnDate::LoadCompressed(const char* compressed, size_t compressed_size,
                                size_t uncompressed_size, size_t row_count) {
    LoadNumbers(data_, compressed, compressed_size, uncompressed_size, row_count);
}

bool ColumnDate::TryGetMinMax(int64_t& min, int64_t& max) const {
    return MinMaxOf(data_, min, max);
}

void ColumnDate::Write(std::ofstream& output) const {
    WriteNumbers(output, data_);
}
