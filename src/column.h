#pragma once

#include <memory>
#include <string>
#include <vector>
#include "file_writer.h"
#include "type.h"
#include "value.h"

class Column {
public:
    virtual ~Column() = default;

    virtual Type GetType() const = 0;

    virtual void PushBack(const std::string& value) = 0;
    virtual void PushBack(const Value& value) = 0;

    virtual size_t Size() const = 0;

    virtual std::string operator[](size_t idx) const = 0;
    virtual Value Get(size_t idx) const = 0;

    virtual void Write(std::ofstream& output) = 0;
};

std::unique_ptr<Column> MakeColumn(Type type);

class ColumnInt64 : public Column {
public:
    Type GetType() const override;

    void PushBack(const std::string& value) override;
    void PushBack(const Value& value) override;

    size_t Size() const override;

    std::string operator[](size_t idx) const override;
    Value Get(size_t idx) const override;

    void Write(std::ofstream& output) override;

private:
    std::vector<int64_t> data_;
};

class ColumnInt128 : public Column {
public:
    Type GetType() const override;

    void PushBack(const std::string& value) override;
    void PushBack(const Value& value) override;

    size_t Size() const override;

    std::string operator[](size_t idx) const override;
    Value Get(size_t idx) const override;

    void Write(std::ofstream& output) override;

private:
    std::vector<__int128> data_;
};

class ColumnString : public Column {
public:
    Type GetType() const override;

    void PushBack(const std::string& value) override;
    void PushBack(const Value& value) override;

    size_t Size() const override;

    std::string operator[](size_t idx) const override;
    Value Get(size_t idx) const override;

    void Write(std::ofstream& output) override;

private:
    std::vector<std::string> data_;
};

class ColumnTimestamp : public Column {
public:
    Type GetType() const override;

    void PushBack(const std::string& value) override;
    void PushBack(const Value& value) override;

    size_t Size() const override;

    std::string operator[](size_t idx) const override;
    Value Get(size_t idx) const override;

    void Write(std::ofstream& output) override;

private:
    std::vector<std::string> data_;
};

class ColumnDate : public Column {
public:
    Type GetType() const override;

    void PushBack(const std::string& value) override;
    void PushBack(const Value& value) override;

    size_t Size() const override;

    std::string operator[](size_t idx) const override;
    Value Get(size_t idx) const override;

    void Write(std::ofstream& output) override;

private:
    std::vector<std::string> data_;
};