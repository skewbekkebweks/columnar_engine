#include "type.h"
#include "error.h"

#include <spdlog/spdlog.h>

Type StringToType(const std::string& str) {
    if (str == "int64") {
        return Type::Int64;
    } else if (str == "int128") {
        return Type::Int128;
    } else if (str == "string") {
        return Type::String;
    } else if (str == "timestamp") {
        return Type::Timestamp;
    } else if (str == "date") {
        return Type::Date;
    }
    THROW_RUNTIME_ERROR("unknown type: " + str);
}

std::string TypeToString(Type type) {
    switch (type) {
        case Type::Int64:
            return "int64";
        case Type::Int128:
            return "int128";
        case Type::String:
            return "string";
        case Type::Timestamp:
            return "timestamp";
        case Type::Date:
            return "date";
        default:
            THROW_NOT_IMPLEMENTED;
    }
}