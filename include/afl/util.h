#pragma once

#include <filesystem>
#include <functional>
#include <hk/types.h>
#include <hk/util/Math.h>
#include <string>
#include <vector>

#include "float16_t/float16_t.hpp"

using f16 = numeric::float16_t;

namespace fs = std::filesystem;

namespace util {

enum class ByteOrder {
	Big,
	Little,
};

u16 bswap16(u16 value);
u32 bswap32(u32 value);

bool isEqual(std::string str1, std::string str2);
u32 roundUp(u32 x, u32 powerOf2);
hk::Result readFile(std::vector<u8>& contents, const fs::path& filename);
void writeFile(const fs::path& filename, const std::vector<u8>& contents);
void writeFile(const fs::path& filename, const std::string& contents);

template <class T>
inline void hashCombine(size_t& s, const T& v) {
	std::hash<T> h;
	s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
}

} // namespace util

template <typename T>
struct std::hash<hk::util::Vector2<T>> {
	std::size_t operator()(const hk::util::Vector2<T>& vec) const noexcept {
		size_t out = 0;
		util::hashCombine(out, vec.x);
		util::hashCombine(out, vec.y);
		return out;
	}
};

template <typename T>
struct std::hash<hk::util::Vector3<T>> {
	std::size_t operator()(const hk::util::Vector3<T>& vec) const noexcept {
		size_t out = 0;
		util::hashCombine(out, vec.x);
		util::hashCombine(out, vec.y);
		util::hashCombine(out, vec.z);
		return out;
	}
};

namespace reader {
u8 readU8(const u8* offset);
s8 readS8(const u8* offset);
u16 readU16(const u8* offset, util::ByteOrder byteOrder);
s16 readS16(const u8* offset, util::ByteOrder byteOrder);
f16 readF16(const u8* offset, util::ByteOrder byteOrder);
u32 readU24(const u8* offset, util::ByteOrder byteOrder);
u32 readU32(const u8* offset, util::ByteOrder byteOrder);
s32 readS32(const u8* offset, util::ByteOrder byteOrder);
f32 readF32(const u8* offset, util::ByteOrder byteOrder);
u64 readU64(const u8* offset, util::ByteOrder byteOrder);
s64 readS64(const u8* offset, util::ByteOrder byteOrder);
f64 readF64(const u8* offset, util::ByteOrder byteOrder);

u16 readU16BE(const u8* offset);
u16 readU16LE(const u8* offset);
s16 readS16BE(const u8* offset);
s16 readS16LE(const u8* offset);
f16 readF16BE(const u8* offset);
f16 readF16LE(const u8* offset);
u32 readU24BE(const u8* offset);
u32 readU24LE(const u8* offset);
u32 readU32BE(const u8* offset);
u32 readU32LE(const u8* offset);
s32 readS32BE(const u8* offset);
s32 readS32LE(const u8* offset);
f32 readF32BE(const u8* offset);
f32 readF32LE(const u8* offset);
u64 readU64BE(const u8* offset);
u64 readU64LE(const u8* offset);
s64 readS64BE(const u8* offset);
s64 readS64LE(const u8* offset);
f64 readF64BE(const u8* offset);
f64 readF64LE(const u8* offset);

hk::Result readByteOrder(util::ByteOrder* out, const u8* offset, u16 expectedBE);
hk::Result checkSignature(const u8* offset, const std::string& expected, size_t length);
const std::string readString(const u8* offset);
const std::string readString(const u8* offset, size_t length);
std::vector<u8> readBytes(const u8* offset, size_t size);
} // namespace reader

namespace writer {
void writeU8(std::vector<u8>& buffer, size_t offset, u8 value);
void writeU16(std::vector<u8>& buffer, size_t offset, u16 value, util::ByteOrder byteOrder);
void writeU24(std::vector<u8>& buffer, size_t offset, u32 value, util::ByteOrder byteOrder);
void writeU32(std::vector<u8>& buffer, size_t offset, u32 value, util::ByteOrder byteOrder);
void writeS32(std::vector<u8>& buffer, size_t offset, s32 value, util::ByteOrder byteOrder);
void writeF32(std::vector<u8>& buffer, size_t offset, f32 value, util::ByteOrder byteOrder);
void writeU64(std::vector<u8>& buffer, size_t offset, u64 value, util::ByteOrder byteOrder);
void writeS64(std::vector<u8>& buffer, size_t offset, s64 value, util::ByteOrder byteOrder);
void writeF64(std::vector<u8>& buffer, size_t offset, f64 value, util::ByteOrder byteOrder);

void writeU16BE(std::vector<u8>& buffer, size_t offset, u16 value);
void writeU16LE(std::vector<u8>& buffer, size_t offset, u16 value);
void writeU24BE(std::vector<u8>& buffer, size_t offset, u32 value);
void writeU24LE(std::vector<u8>& buffer, size_t offset, u32 value);
void writeU32BE(std::vector<u8>& buffer, size_t offset, u32 value);
void writeU32LE(std::vector<u8>& buffer, size_t offset, u32 value);
void writeS32BE(std::vector<u8>& buffer, size_t offset, s32 value);
void writeS32LE(std::vector<u8>& buffer, size_t offset, s32 value);
void writeF32BE(std::vector<u8>& buffer, size_t offset, f32 value);
void writeF32LE(std::vector<u8>& buffer, size_t offset, f32 value);
void writeU64BE(std::vector<u8>& buffer, size_t offset, u64 value);
void writeU64LE(std::vector<u8>& buffer, size_t offset, u64 value);
void writeS64BE(std::vector<u8>& buffer, size_t offset, s64 value);
void writeS64LE(std::vector<u8>& buffer, size_t offset, s64 value);
void writeF64BE(std::vector<u8>& buffer, size_t offset, f64 value);
void writeF64LE(std::vector<u8>& buffer, size_t offset, f64 value);

void writeString(
	std::vector<u8>& buffer, size_t offset, const std::string& str, bool isNullTerminated = true
);
void writeBytes(std::vector<u8>& buffer, size_t offset, const std::vector<u8>& bytes);
} // namespace writer
