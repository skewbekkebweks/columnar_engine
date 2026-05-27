#pragma once

#include <vector>
#include <cstddef>

#include <lz4.h>

#include "error.h"

inline std::vector<char> CompressLz4(const char* data, size_t size) {
    int bound = LZ4_compressBound(size);
    std::vector<char> dst(bound);
    int compressed_size = LZ4_compress_default(data, dst.data(), size, bound);
    dst.resize(compressed_size);
    return dst;
}

inline std::vector<char> DecompressLz4(const char* data, size_t compressed_size,
                                        size_t uncompressed_size) {
    std::vector<char> dst(uncompressed_size);
    LZ4_decompress_safe(data, dst.data(), compressed_size, uncompressed_size);
    return dst;
}
