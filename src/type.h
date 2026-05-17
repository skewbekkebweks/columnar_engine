#pragma once

#include <string>

enum class Type {
    Int64,
    Int128,
    String,
};

Type StringToType(const std::string&);
std::string TypeToString(Type);