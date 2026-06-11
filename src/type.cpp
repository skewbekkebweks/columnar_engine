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
