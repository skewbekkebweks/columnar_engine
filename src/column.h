#pragma once

#include <string>
#include <vector>
#include "type.h"
#include "file_writer.h"


class Column {
public:
    virtual Type GetType() const = 0;

    virtual void PushBack(const std::string& value) = 0;

    virtual size_t Size() const = 0;

    virtual std::string operator[](size_t idx) = 0;

    virtual void Write(std::ofstream& output) = 0;
};

class ColumnInt64 : public Column {
public:
    Type GetType() const override;

    void PushBack(const std::string& value) override;

    size_t Size() const override;

    std::string operator[](size_t idx) override;

    void Write(std::ofstream& output);
private:
    std::vector<int64_t> data_;
};

class ColumnString : public Column {
public:
    Type GetType() const override;
    
    void PushBack(const std::string& value) override;

    size_t Size() const override;
    
    std::string operator[](size_t idx) override;

    void Write(std::ofstream& output);
private:
    std::vector<std::string> data_;
};