#include "type.h"
#include "error.h"

#include <spdlog/spdlog.h>

Type StringToType(const std::string& str) {
    if (str == "int64") {
        return Type::Int64;
    } else if (str == "string") {
        return Type::String;
    }
    THROW_RUNTIME_ERROR("unknown type: " + str);
}

std::string TypeToString(Type type) {
    switch (type) {
        case Type::Int64: {
            return "int64";
        }
        case Type::String: {
            return "string";
        }
        default:
            THROW_NOT_IMPLEMENTED;
    }
}