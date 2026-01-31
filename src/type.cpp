#include "type.h"

#include <stdexcept>
#include <spdlog/spdlog.h>

Type StringToType(std::string str) {
    if (str == "int64") {
        return Type::Int64;
    } else if (str == "string") {
        return Type::String;
    }
    throw std::runtime_error{"unknown type: " + str};
}

std::string TypeToString(Type type) {
    switch (type) {
        case Type::Int64: {
            return "int64";
        }
        case Type::String: {
            return "string";
        }
        default: {
            spdlog::error("Unreachable code reached in TypeToString");
            throw std::runtime_error{"Unreachable code reached in TypeToString"};
        }
    }
}