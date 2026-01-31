#pragma once

#include <fstream>
#include <concepts>

#include "spdlog/spdlog.h"

template <typename T>
T Read(std::ifstream& input) {
    T value;
    if constexpr (std::integral<T>) {
        input.read(reinterpret_cast<char*>(&value), sizeof(value));
    } else if constexpr (std::same_as<T, std::string>) {
        std::getline(input, value, '\0');
    } else {
        spdlog::error("Read is not implemented for your type");
    }
    return value;
}