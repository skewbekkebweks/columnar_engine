#pragma once

#include <string>

enum class Type {
    Int64,
    String,
};

Type StringToType(std::string);
std::string TypeToString(Type);