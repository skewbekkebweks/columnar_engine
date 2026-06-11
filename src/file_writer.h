#pragma once

#include <fstream>
#include <concepts>

#include "spdlog/spdlog.h"

template <typename T>
void Write(std::ofstream& output, const T& value) {
    if constexpr (std::integral<T>) {
        output.write(reinterpret_cast<const char*>(&value), sizeof(value));
    } else if constexpr (std::same_as<T, std::string>) {
        output.write(value.data(), value.size());
        output.write("\0", 1);
    } else {
        spdlog::error("Write is not implemented for your type");
    }
}