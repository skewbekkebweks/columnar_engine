#pragma once

#include <string>

enum class Type {
    Int64,
    String,
};

Type StringToType(const std::string&);
std::string TypeToString(Type);