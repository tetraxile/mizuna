#pragma once

// Yaz0 compression file format
// credit to http://amnoid.de/gc/yaz0.txt for helping me understand the format

#include <hk/types.h>
#include <vector>

namespace yaz0 {
hk::Result decompress(std::vector<u8>& output, const std::vector<u8>& input);
void compress(std::vector<u8>& output, const std::vector<u8>& input, u32 alignment);
} // namespace yaz0
