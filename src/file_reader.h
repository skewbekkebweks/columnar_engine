#pragma once

#include <concepts>
#include <string>

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

template <typename T>
T ReadFromBuffer(const char*& p) {
    if constexpr (std::integral<T>) {
        T value;
        std::memcpy(&value, p, sizeof(T));
        p += sizeof(T);
        return value;
    } else if constexpr (std::same_as<T, std::string>) {
        std::string value(p);
        p += value.size() + 1;
        return value;
    } else {
        spdlog::error("ReadFromBuffer is not implemented for your type");
        return T{};
    }
}