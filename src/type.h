#pragma once

#include <string>

enum class Type {
    Int64,
    Int128,
    String,
    Timestamp,
    Date,
};

Type StringToType(const std::string&);