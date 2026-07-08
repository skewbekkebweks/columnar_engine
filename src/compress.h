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

inline void DecompressLz4(const char* data, size_t compressed_size, char* dst,
                          size_t uncompressed_size) {
    LZ4_decompress_safe(data, dst, static_cast<int>(compressed_size),
                        static_cast<int>(uncompressed_size));
}

inline std::vector<char> DecompressLz4(const char* data, size_t compressed_size,
                                       size_t uncompressed_size) {
    std::vector<char> dst(uncompressed_size);
    DecompressLz4(data, compressed_size, dst.data(), uncompressed_size);
    return dst;
}
