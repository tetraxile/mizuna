#include "mizuna/util.h"

#include <bit>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>

#include "mizuna/results.h"

namespace util {
u16 bswap16(u16 value) {
	return ((value & 0xff) << 8) | ((value & 0xff00) >> 8);
}

u32 bswap32(u32 value) {
	return ((value & 0x000000ff) << 24) | ((value & 0x0000ff00) << 8) |
	       ((value & 0x00ff0000) >> 8) | ((value & 0xff000000) >> 24);
}

bool isEqual(std::string str1, std::string str2) {
	return std::strcmp(str1.c_str(), str2.c_str()) == 0;
}

u32 roundUp(u32 x, u32 powerOf2) {
	u32 a = powerOf2 - 1;
	return (x + a) & ~a;
}

hk::Result readFile(std::vector<u8>& contents, const fs::path& filename) {
	std::ifstream fstream(filename, std::ios::binary);

	if (fstream.eof() || fstream.fail()) {
		return ResultFileError();
	}

	// disable skipping whitespace in binary file
	fstream.unsetf(std::ios::skipws);

	fstream.seekg(0, std::ios_base::end);
	std::streampos fileSize = fstream.tellg();
	fstream.seekg(0, std::ios_base::beg);

	contents.reserve(fileSize);
	contents.insert(
		contents.begin(), std::istream_iterator<u8>(fstream), std::istream_iterator<u8>()
	);

	fstream.close();

	return hk::ResultSuccess();
}

void writeFile(const fs::path& filename, const std::vector<u8>& contents) {
	std::ofstream fstream(filename, std::ios::out | std::ios::binary);
	fstream.write(reinterpret_cast<const char*>(contents.data()), contents.size());
}

void writeFile(const fs::path& filename, const std::string& contents) {
	std::ofstream fstream(filename, std::ios::out);
	fstream.write(contents.c_str(), contents.size());
}

} // namespace util

namespace reader {
hk::Result readByteOrder(util::ByteOrder* out, const u8* offset, u16 expectedBE) {
	u16 mark = (offset[0] << 8) | offset[1];
	if (mark == expectedBE)
		*out = util::ByteOrder::Big;
	else if (mark == util::bswap16(expectedBE))
		*out = util::ByteOrder::Little;
	else
		return ResultBadByteOrder();

	return hk::ResultSuccess();
}

hk::Result checkSignature(const u8* offset, const std::string& expected, size_t length) {
	for (size_t i = 0; i < length; i++)
		if (offset[i] != expected[i]) return ResultBadSignature();

	return hk::ResultSuccess();
}

u8 readU8(const u8* offset) {
	return *offset;
}

s8 readS8(const u8* offset) {
	return std::bit_cast<s8>(readU8(offset));
}

u16 readU16BE(const u8* offset) {
	return (offset[0] << 8) | (offset[1] << 0);
}

u16 readU16LE(const u8* offset) {
	return (offset[0] << 0) | (offset[1] << 8);
}

u16 readU16(const u8* offset, util::ByteOrder byteOrder) {
	if (byteOrder == util::ByteOrder::Big)
		return readU16BE(offset);
	else
		return readU16LE(offset);
}

s16 readS16BE(const u8* offset) {
	return std::bit_cast<s16>(readU16BE(offset));
}

s16 readS16LE(const u8* offset) {
	return std::bit_cast<s16>(readU16LE(offset));
}

s16 readS16(const u8* offset, util::ByteOrder byteOrder) {
	return std::bit_cast<s16>(readU16(offset, byteOrder));
}

f16 readF16BE(const u8* offset) {
	return std::bit_cast<f16>(readU16BE(offset));
}

f16 readF16LE(const u8* offset) {
	return std::bit_cast<f16>(readU16LE(offset));
}

f16 readF16(const u8* offset, util::ByteOrder byteOrder) {
	return std::bit_cast<f16>(readU16(offset, byteOrder));
}

u32 readU24BE(const u8* offset) {
	return (offset[0] << 16) | (offset[1] << 8) | (offset[2] << 0);
}

u32 readU24LE(const u8* offset) {
	return (offset[0] << 0) | (offset[1] << 8) | (offset[2] << 16);
}

u32 readU24(const u8* offset, util::ByteOrder byteOrder) {
	if (byteOrder == util::ByteOrder::Big)
		return readU24BE(offset);
	else
		return readU24LE(offset);
}

u32 readU32BE(const u8* offset) {
	return (offset[0] << 24) | (offset[1] << 16) | (offset[2] << 8) | offset[3];
}

u32 readU32LE(const u8* offset) {
	return offset[0] | (offset[1] << 8) | (offset[2] << 16) | (offset[3] << 24);
}

u32 readU32(const u8* offset, util::ByteOrder byteOrder) {
	if (byteOrder == util::ByteOrder::Big)
		return readU32BE(offset);
	else
		return readU32LE(offset);
}

s32 readS32BE(const u8* offset) {
	return std::bit_cast<s32>(readU32BE(offset));
}

s32 readS32LE(const u8* offset) {
	return std::bit_cast<s32>(readU32LE(offset));
}

s32 readS32(const u8* offset, util::ByteOrder byteOrder) {
	return std::bit_cast<s32>(readU32(offset, byteOrder));
}

f32 readF32BE(const u8* offset) {
	return std::bit_cast<f32>(readU32BE(offset));
}

f32 readF32LE(const u8* offset) {
	return std::bit_cast<f32>(readU32LE(offset));
}

f32 readF32(const u8* offset, util::ByteOrder byteOrder) {
	return std::bit_cast<f32>(readU32(offset, byteOrder));
}

u64 readU64BE(const u8* offset) {
	return ((u64)offset[0] << 56) | ((u64)offset[1] << 48) | ((u64)offset[2] << 40) |
	       ((u64)offset[3] << 32) | ((u64)offset[4] << 24) | ((u64)offset[5] << 16) |
	       ((u64)offset[6] << 8) | ((u64)offset[7] << 0);
}

u64 readU64LE(const u8* offset) {
	return ((u64)offset[0] << 0) | ((u64)offset[1] << 8) | ((u64)offset[2] << 16) |
	       ((u64)offset[3] << 24) | ((u64)offset[4] << 32) | ((u64)offset[5] << 40) |
	       ((u64)offset[6] << 48) | ((u64)offset[7] << 56);
}

u64 readU64(const u8* offset, util::ByteOrder byteOrder) {
	if (byteOrder == util::ByteOrder::Big)
		return readU64BE(offset);
	else
		return readU64LE(offset);
}

s64 readS64BE(const u8* offset) {
	return std::bit_cast<s64>(readU64BE(offset));
}

s64 readS64LE(const u8* offset) {
	return std::bit_cast<s64>(readU64LE(offset));
}

s64 readS64(const u8* offset, util::ByteOrder byteOrder) {
	return std::bit_cast<s64>(readU64(offset, byteOrder));
}

f64 readF64BE(const u8* offset) {
	return std::bit_cast<f64>(readU64BE(offset));
}

f64 readF64LE(const u8* offset) {
	return std::bit_cast<f64>(readU64LE(offset));
}

f64 readF64(const u8* offset, util::ByteOrder byteOrder) {
	return std::bit_cast<f64>(readU64(offset, byteOrder));
}

const std::string readString(const u8* offset) {
	std::string str((const char*)offset);
	return str;
}

const std::string readString(const u8* offset, size_t length) {
	std::string str((const char*)offset, length);
	return str;
}

std::vector<u8> readBytes(const u8* offset, size_t size) {
	std::vector<u8> slice(offset, offset + size);
	return slice;
}
} // namespace reader

namespace writer {
void writeU8(std::vector<u8>& buffer, size_t offset, u8 value) {
	if (offset + 1 > buffer.size()) buffer.resize(offset + 1);

	buffer[offset] = value;
}

void writeU16BE(std::vector<u8>& buffer, size_t offset, u16 value) {
	if (offset + 2 > buffer.size()) buffer.resize(offset + 2);

	buffer[offset + 0] = value >> 8 & 0xff;
	buffer[offset + 1] = value >> 0 & 0xff;
}

void writeU16LE(std::vector<u8>& buffer, size_t offset, u16 value) {
	if (offset + 2 > buffer.size()) buffer.resize(offset + 2);

	buffer[offset + 0] = value >> 0 & 0xff;
	buffer[offset + 1] = value >> 8 & 0xff;
}

void writeU16(std::vector<u8>& buffer, size_t offset, u16 value, util::ByteOrder byteOrder) {
	if (byteOrder == util::ByteOrder::Big)
		writeU16BE(buffer, offset, value);
	else
		writeU16LE(buffer, offset, value);
}

void writeU24BE(std::vector<u8>& buffer, size_t offset, u32 value) {
	if (offset + 3 > buffer.size()) buffer.resize(offset + 3);

	buffer[offset + 0] = value >> 16 & 0xff;
	buffer[offset + 1] = value >> 8 & 0xff;
	buffer[offset + 2] = value >> 0 & 0xff;
}

void writeU24LE(std::vector<u8>& buffer, size_t offset, u32 value) {
	if (offset + 3 > buffer.size()) buffer.resize(offset + 3);

	buffer[offset + 0] = value >> 0 & 0xff;
	buffer[offset + 1] = value >> 8 & 0xff;
	buffer[offset + 2] = value >> 16 & 0xff;
}

void writeU24(std::vector<u8>& buffer, size_t offset, u32 value, util::ByteOrder byteOrder) {
	if (byteOrder == util::ByteOrder::Big)
		writeU24BE(buffer, offset, value);
	else
		writeU24LE(buffer, offset, value);
}

void writeU32BE(std::vector<u8>& buffer, size_t offset, u32 value) {
	if (offset + 4 > buffer.size()) buffer.resize(offset + 4);

	buffer[offset + 0] = value >> 24 & 0xff;
	buffer[offset + 1] = value >> 16 & 0xff;
	buffer[offset + 2] = value >> 8 & 0xff;
	buffer[offset + 3] = value >> 0 & 0xff;
}

void writeU32LE(std::vector<u8>& buffer, size_t offset, u32 value) {
	if (offset + 4 > buffer.size()) buffer.resize(offset + 4);

	buffer[offset + 0] = value >> 0 & 0xff;
	buffer[offset + 1] = value >> 8 & 0xff;
	buffer[offset + 2] = value >> 16 & 0xff;
	buffer[offset + 3] = value >> 24 & 0xff;
}

void writeU32(std::vector<u8>& buffer, size_t offset, u32 value, util::ByteOrder byteOrder) {
	if (byteOrder == util::ByteOrder::Big)
		writeU32BE(buffer, offset, value);
	else
		writeU32LE(buffer, offset, value);
}

void writeS32BE(std::vector<u8>& buffer, size_t offset, s32 value) {
	writeU32BE(buffer, offset, std::bit_cast<u32>(value));
}

void writeS32LE(std::vector<u8>& buffer, size_t offset, s32 value) {
	writeU32LE(buffer, offset, std::bit_cast<u32>(value));
}

void writeS32(std::vector<u8>& buffer, size_t offset, s32 value, util::ByteOrder byteOrder) {
	writeU32(buffer, offset, std::bit_cast<u32>(value), byteOrder);
}

void writeF32BE(std::vector<u8>& buffer, size_t offset, f32 value) {
	writeU32BE(buffer, offset, std::bit_cast<u32>(value));
}

void writeF32LE(std::vector<u8>& buffer, size_t offset, f32 value) {
	writeU32LE(buffer, offset, std::bit_cast<u32>(value));
}

void writeF32(std::vector<u8>& buffer, size_t offset, f32 value, util::ByteOrder byteOrder) {
	writeU32(buffer, offset, std::bit_cast<u32>(value), byteOrder);
}

void writeU64BE(std::vector<u8>& buffer, size_t offset, u64 value) {
	if (offset + 8 > buffer.size()) buffer.resize(offset + 8);

	buffer[offset + 0] = value >> 56 & 0xff;
	buffer[offset + 1] = value >> 48 & 0xff;
	buffer[offset + 2] = value >> 40 & 0xff;
	buffer[offset + 3] = value >> 32 & 0xff;
	buffer[offset + 4] = value >> 24 & 0xff;
	buffer[offset + 5] = value >> 16 & 0xff;
	buffer[offset + 6] = value >> 8 & 0xff;
	buffer[offset + 7] = value >> 0 & 0xff;
}

void writeU64LE(std::vector<u8>& buffer, size_t offset, u64 value) {
	if (offset + 8 > buffer.size()) buffer.resize(offset + 8);

	buffer[offset + 0] = value >> 0 & 0xff;
	buffer[offset + 1] = value >> 8 & 0xff;
	buffer[offset + 2] = value >> 16 & 0xff;
	buffer[offset + 3] = value >> 24 & 0xff;
	buffer[offset + 4] = value >> 32 & 0xff;
	buffer[offset + 5] = value >> 40 & 0xff;
	buffer[offset + 6] = value >> 48 & 0xff;
	buffer[offset + 7] = value >> 56 & 0xff;
}

void writeU64(std::vector<u8>& buffer, size_t offset, u64 value, util::ByteOrder byteOrder) {
	if (byteOrder == util::ByteOrder::Big)
		writeU64BE(buffer, offset, value);
	else
		writeU64LE(buffer, offset, value);
}

void writeS64BE(std::vector<u8>& buffer, size_t offset, s64 value) {
	writeU64BE(buffer, offset, std::bit_cast<u64>(value));
}

void writeS64LE(std::vector<u8>& buffer, size_t offset, s64 value) {
	writeU64LE(buffer, offset, std::bit_cast<u64>(value));
}

void writeS64(std::vector<u8>& buffer, size_t offset, s64 value, util::ByteOrder byteOrder) {
	writeU64(buffer, offset, std::bit_cast<u64>(value), byteOrder);
}

void writeF64BE(std::vector<u8>& buffer, size_t offset, f64 value) {
	writeU64BE(buffer, offset, std::bit_cast<u64>(value));
}

void writeF64LE(std::vector<u8>& buffer, size_t offset, f64 value) {
	writeU64LE(buffer, offset, std::bit_cast<u64>(value));
}

void writeF64(std::vector<u8>& buffer, size_t offset, f64 value, util::ByteOrder byteOrder) {
	writeU64(buffer, offset, std::bit_cast<u64>(value), byteOrder);
}

void writeString(
	std::vector<u8>& buffer, size_t offset, const std::string& str, bool isNullTerminated
) {
	for (size_t i = 0; i < str.size(); i++)
		writeU8(buffer, offset + i, str[i]);
	if (isNullTerminated) writeU8(buffer, offset + str.size(), 0);
}

void writeBytes(std::vector<u8>& buffer, size_t offset, const std::vector<u8>& bytes) {
	for (size_t i = 0; i < bytes.size(); i++)
		writeU8(buffer, offset + i, bytes[i]);
}

} // namespace writer
