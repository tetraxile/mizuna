#include "afl/bfres/reader.h"
#include "afl/bfres/results.h"

namespace bfres {

hk::Result BufferInfo::read(const u8* offset) {
	u32 unk = reader::readU32(offset, mByteOrder);
	mBufferSize = reader::readU32(offset + 0x4, mByteOrder);
	mBufferOffset = reader::readU64(offset + 0x8, mByteOrder);

	return hk::ResultSuccess();
}

hk::Result DataNode::readHeader(const u8* offset, const std::string& signature) {
	HK_TRY(reader::checkSignature(offset, signature, signature.length()));

	u32 nextBlockOffset = reader::readU32(offset + 0x4, mByteOrder);
	u32 blockSize = reader::readU32(offset + 0x8, mByteOrder);

	// if (blockSize != size() && blockSize != 0) {
	// 	printf("info: %s block size %x (expected %x)\n", signature.c_str(), blockSize, size());
	// }

	return hk::ResultSuccess();
}

std::string DataNode::readString(const u8* offset) {
	u64 strOffset = reader::readU64(offset, mByteOrder);
	u16 len = reader::readU16(mBase + strOffset, mByteOrder);
	return reader::readString(mBase + strOffset + 2, len);
}

u32 getIndexFormatStride(IndexFormat fmt) {
	switch (fmt) {
	case IndexFormat::U8: return 1;
	case IndexFormat::U16: return 2;
	case IndexFormat::U32: return 3;
	default: return 0;
	}
}

u32 getAttrFormatStride(AttributeFormat fmt) {
	switch (fmt) {
	case AttributeFormat::Format_8_UNorm:
	case AttributeFormat::Format_8_UInt:
	case AttributeFormat::Format_8_SNorm:
	case AttributeFormat::Format_8_SInt:
	case AttributeFormat::Format_8_UIntToSingle:
	case AttributeFormat::Format_8_SIntToSingle:
	case AttributeFormat::Format_4_4_UNorm: return 1;

	case AttributeFormat::Format_16_UNorm:
	case AttributeFormat::Format_16_UInt:
	case AttributeFormat::Format_16_SNorm:
	case AttributeFormat::Format_16_SInt:
	case AttributeFormat::Format_16_Single:
	case AttributeFormat::Format_16_UIntToSingle:
	case AttributeFormat::Format_16_SIntToSingle:
	case AttributeFormat::Format_8_8_UNorm:
	case AttributeFormat::Format_8_8_UInt:
	case AttributeFormat::Format_8_8_SNorm:
	case AttributeFormat::Format_8_8_SInt:
	case AttributeFormat::Format_8_8_UIntToSingle:
	case AttributeFormat::Format_8_8_SIntToSingle: return 2;

	case AttributeFormat::Format_16_16_UNorm:
	case AttributeFormat::Format_16_16_SNorm:
	case AttributeFormat::Format_16_16_UInt:
	case AttributeFormat::Format_16_16_SInt:
	case AttributeFormat::Format_16_16_Single:
	case AttributeFormat::Format_16_16_UIntToSingle:
	case AttributeFormat::Format_16_16_SIntToSingle:
	case AttributeFormat::Format_10_11_11_Single:
	case AttributeFormat::Format_8_8_8_8_UNorm:
	case AttributeFormat::Format_8_8_8_8_SNorm:
	case AttributeFormat::Format_8_8_8_8_UInt:
	case AttributeFormat::Format_8_8_8_8_SInt:
	case AttributeFormat::Format_8_8_8_8_UIntToSingle:
	case AttributeFormat::Format_8_8_8_8_SIntToSingle:
	case AttributeFormat::Format_10_10_10_2_UNorm:
	case AttributeFormat::Format_10_10_10_2_UInt:
	case AttributeFormat::Format_10_10_10_2_SNorm:
	case AttributeFormat::Format_10_10_10_2_SInt:
	case AttributeFormat::Format_32_UInt:
	case AttributeFormat::Format_32_SInt:
	case AttributeFormat::Format_32_Single: return 4;

	case AttributeFormat::Format_16_16_16_16_UNorm:
	case AttributeFormat::Format_16_16_16_16_SNorm:
	case AttributeFormat::Format_16_16_16_16_UInt:
	case AttributeFormat::Format_16_16_16_16_SInt:
	case AttributeFormat::Format_16_16_16_16_Single:
	case AttributeFormat::Format_16_16_16_16_UIntToSingle:
	case AttributeFormat::Format_16_16_16_16_SIntToSingle:
	case AttributeFormat::Format_32_32_UInt:
	case AttributeFormat::Format_32_32_SInt:
	case AttributeFormat::Format_32_32_Single: return 8;

	case AttributeFormat::Format_32_32_32_UInt:
	case AttributeFormat::Format_32_32_32_SInt:
	case AttributeFormat::Format_32_32_32_Single: return 12;

	case AttributeFormat::Format_32_32_32_32_UInt:
	case AttributeFormat::Format_32_32_32_32_SInt:
	case AttributeFormat::Format_32_32_32_32_Single: return 16;

	case AttributeFormat::None:
	default: return 0;
	}
}

u32 getAttrFormatSize(AttributeFormat fmt) {
	switch (fmt) {
	case AttributeFormat::Format_8_UNorm:
	case AttributeFormat::Format_8_UInt:
	case AttributeFormat::Format_8_SNorm:
	case AttributeFormat::Format_8_SInt:
	case AttributeFormat::Format_8_UIntToSingle:
	case AttributeFormat::Format_8_SIntToSingle:
	case AttributeFormat::Format_16_UNorm:
	case AttributeFormat::Format_16_UInt:
	case AttributeFormat::Format_16_SNorm:
	case AttributeFormat::Format_16_SInt:
	case AttributeFormat::Format_16_Single:
	case AttributeFormat::Format_16_UIntToSingle:
	case AttributeFormat::Format_16_SIntToSingle:
	case AttributeFormat::Format_32_UInt:
	case AttributeFormat::Format_32_SInt:
	case AttributeFormat::Format_32_Single: return 1;

	case AttributeFormat::Format_4_4_UNorm:
	case AttributeFormat::Format_8_8_UNorm:
	case AttributeFormat::Format_8_8_UInt:
	case AttributeFormat::Format_8_8_SNorm:
	case AttributeFormat::Format_8_8_SInt:
	case AttributeFormat::Format_8_8_UIntToSingle:
	case AttributeFormat::Format_8_8_SIntToSingle:
	case AttributeFormat::Format_16_16_UNorm:
	case AttributeFormat::Format_16_16_SNorm:
	case AttributeFormat::Format_16_16_UInt:
	case AttributeFormat::Format_16_16_SInt:
	case AttributeFormat::Format_16_16_Single:
	case AttributeFormat::Format_16_16_UIntToSingle:
	case AttributeFormat::Format_16_16_SIntToSingle:
	case AttributeFormat::Format_32_32_UInt:
	case AttributeFormat::Format_32_32_SInt:
	case AttributeFormat::Format_32_32_Single: return 2;

	case AttributeFormat::Format_10_10_10_2_UNorm:
	case AttributeFormat::Format_10_10_10_2_UInt:
	case AttributeFormat::Format_10_10_10_2_SNorm:
	case AttributeFormat::Format_10_10_10_2_SInt:
	case AttributeFormat::Format_32_32_32_UInt:
	case AttributeFormat::Format_32_32_32_SInt:
	case AttributeFormat::Format_32_32_32_Single:
	case AttributeFormat::Format_10_11_11_Single: return 3;

	case AttributeFormat::Format_8_8_8_8_UNorm:
	case AttributeFormat::Format_8_8_8_8_SNorm:
	case AttributeFormat::Format_8_8_8_8_UInt:
	case AttributeFormat::Format_8_8_8_8_SInt:
	case AttributeFormat::Format_8_8_8_8_UIntToSingle:
	case AttributeFormat::Format_8_8_8_8_SIntToSingle:
	case AttributeFormat::Format_16_16_16_16_UNorm:
	case AttributeFormat::Format_16_16_16_16_SNorm:
	case AttributeFormat::Format_16_16_16_16_UInt:
	case AttributeFormat::Format_16_16_16_16_SInt:
	case AttributeFormat::Format_16_16_16_16_Single:
	case AttributeFormat::Format_16_16_16_16_UIntToSingle:
	case AttributeFormat::Format_16_16_16_16_SIntToSingle:
	case AttributeFormat::Format_32_32_32_32_UInt:
	case AttributeFormat::Format_32_32_32_32_SInt:
	case AttributeFormat::Format_32_32_32_32_Single: return 4;

	case AttributeFormat::None:
	default: return 0;
	}
}

hk::Result readAttrFormat(
	hk::util::Vector4f* out, const u8* offset, AttributeFormat fmt, util::ByteOrder byteOrder
) {
	*out = { 0, 0, 0, 0 };

	switch (fmt) {
	case AttributeFormat::Format_8_UNorm: {
		out->x = (f32)reader::readU8(offset) / 255.0f;
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_8_UInt: {
		out->x = (f32)reader::readU8(offset);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_8_SNorm: {
		out->x = (f32)reader::readS8(offset) / 127.0f;
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_8_SInt: {
		out->x = (f32)reader::readS8(offset);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_8_UIntToSingle: {
		out->x = (f32)reader::readU8(offset);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_8_SIntToSingle: {
		out->x = (f32)reader::readS8(offset);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_4_4_UNorm: {
		u32 p = reader::readU8(offset);
		out->x = (f32)(p & 0xf) / 16.0f;
		out->y = (f32)((p >> 4) & 0xf) / 16.0f;
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_16_UNorm: {
		out->x = (f32)reader::readU16(offset, byteOrder) / 65535.0f;
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_16_UInt: {
		out->x = (f32)reader::readU16(offset, byteOrder);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_16_SNorm: {
		out->x = (f32)reader::readS16(offset, byteOrder) / 32767.0f;
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_16_SInt: {
		out->x = (f32)reader::readS16(offset, byteOrder);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_16_Single: {
		out->x = (f32)reader::readF16(offset, byteOrder);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_16_UIntToSingle: {
		out->x = (f32)reader::readU16(offset, byteOrder);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_16_SIntToSingle: {
		out->x = (f32)reader::readS16(offset, byteOrder);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_8_8_UNorm: {
		out->x = (f32)reader::readU8(offset) / 255.0f;
		out->y = (f32)reader::readU8(offset + 1) / 255.0f;
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_8_8_UInt: {
		out->x = (f32)reader::readU8(offset);
		out->y = (f32)reader::readU8(offset + 1);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_8_8_SNorm: {
		out->x = (f32)reader::readS8(offset) / 127.0f;
		out->y = (f32)reader::readS8(offset + 1) / 127.0f;
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_8_8_SInt: {
		out->x = (f32)reader::readS8(offset);
		out->y = (f32)reader::readS8(offset + 1);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_8_8_UIntToSingle: {
		out->x = (f32)reader::readU8(offset);
		out->y = (f32)reader::readU8(offset + 1);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_8_8_SIntToSingle: {
		out->x = (f32)reader::readS8(offset);
		out->y = (f32)reader::readS8(offset + 1);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_16_16_UNorm: {
		out->x = (f32)reader::readU16(offset, byteOrder) / 65535.0f;
		out->y = (f32)reader::readU16(offset + 2, byteOrder) / 65535.0f;
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_16_16_UInt: {
		out->x = (f32)reader::readU16(offset, byteOrder);
		out->y = (f32)reader::readU16(offset + 2, byteOrder);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_16_16_SNorm: {
		out->x = (f32)reader::readS16(offset, byteOrder) / 32767.0f;
		out->y = (f32)reader::readS16(offset + 2, byteOrder) / 32767.0f;
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_16_16_SInt: {
		out->x = (f32)reader::readS16(offset, byteOrder);
		out->y = (f32)reader::readS16(offset + 2, byteOrder);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_16_16_Single: {
		out->x = (f32)reader::readF16(offset, byteOrder);
		out->y = (f32)reader::readF16(offset + 2, byteOrder);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_16_16_UIntToSingle: {
		out->x = (f32)reader::readU16(offset, byteOrder);
		out->y = (f32)reader::readU16(offset + 2, byteOrder);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_16_16_SIntToSingle: {
		out->x = (f32)reader::readS16(offset, byteOrder);
		out->y = (f32)reader::readS16(offset + 2, byteOrder);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_10_11_11_Single: {
		fprintf(stderr, "error: unsupported vertex attribute format (10_11_11_Single)");
		return ResultInvalidVertexAttribute();
	}

	case AttributeFormat::Format_8_8_8_8_UNorm: {
		out->x = (f32)reader::readU8(offset) / 255.0f;
		out->y = (f32)reader::readU8(offset + 1) / 255.0f;
		out->z = (f32)reader::readU8(offset + 2) / 255.0f;
		out->w = (f32)reader::readU8(offset + 3) / 255.0f;
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_8_8_8_8_UInt: {
		out->x = (f32)reader::readU8(offset);
		out->y = (f32)reader::readU8(offset + 1);
		out->z = (f32)reader::readU8(offset + 2);
		out->w = (f32)reader::readU8(offset + 3);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_8_8_8_8_SNorm: {
		out->x = (f32)reader::readS8(offset) / 127.0f;
		out->y = (f32)reader::readS8(offset + 1) / 127.0f;
		out->z = (f32)reader::readS8(offset + 2) / 127.0f;
		out->w = (f32)reader::readS8(offset + 3) / 127.0f;
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_8_8_8_8_SInt: {
		out->x = (f32)reader::readS8(offset);
		out->y = (f32)reader::readS8(offset + 1);
		out->z = (f32)reader::readS8(offset + 2);
		out->w = (f32)reader::readS8(offset + 3);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_8_8_8_8_UIntToSingle: {
		out->x = (f32)reader::readU8(offset);
		out->y = (f32)reader::readU8(offset + 1);
		out->z = (f32)reader::readU8(offset + 2);
		out->w = (f32)reader::readU8(offset + 3);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_8_8_8_8_SIntToSingle: {
		out->x = (f32)reader::readS8(offset);
		out->y = (f32)reader::readS8(offset + 1);
		out->z = (f32)reader::readS8(offset + 2);
		out->w = (f32)reader::readS8(offset + 3);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_10_10_10_2_UNorm: {
		u32 p = reader::readU32(offset, byteOrder);
		out->x = (f32)(p & 0x3ff) / 1024.0f;
		out->y = (f32)((p >> 10) & 0x3ff) / 1024.0f;
		out->z = (f32)((p >> 20) & 0x3ff) / 1024.0f;
		out->w = p >> 30;
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_10_10_10_2_UInt: {
		u32 p = reader::readU32(offset, byteOrder);
		out->x = (f32)(p & 0x3ff);
		out->y = (f32)((p >> 10) & 0x3ff);
		out->z = (f32)((p >> 20) & 0x3ff);
		out->w = p >> 30;
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_10_10_10_2_SNorm: {
		struct {
			signed int x : 10;
		} s;

		u32 p = reader::readS32(offset, byteOrder);
		out->x = (f32)(s.x = p & 0x3ff) / 511.0f;
		out->y = (f32)(s.x = (p >> 10) & 0x3ff) / 511.0f;
		out->z = (f32)(s.x = (p >> 20) & 0x3ff) / 511.0f;
		out->w = p >> 30;
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_10_10_10_2_SInt: {
		struct {
			signed int x : 10;
		} s;

		u32 p = reader::readS32(offset, byteOrder);
		out->x = (f32)(s.x = p & 0x3ff);
		out->y = (f32)(s.x = (p >> 10) & 0x3ff);
		out->z = (f32)(s.x = (p >> 20) & 0x3ff);
		out->w = p >> 30;
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_32_UInt: {
		out->x = (f32)reader::readU32(offset, byteOrder);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_32_SInt: {
		out->x = (f32)reader::readS32(offset, byteOrder);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_32_Single: {
		out->x = reader::readF32(offset, byteOrder);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_16_16_16_16_UNorm: {
		out->x = (f32)reader::readU16(offset, byteOrder) / 65535.0f;
		out->y = (f32)reader::readU16(offset + 2, byteOrder) / 65535.0f;
		out->z = (f32)reader::readU16(offset + 4, byteOrder) / 65535.0f;
		out->w = (f32)reader::readU16(offset + 6, byteOrder) / 65535.0f;
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_16_16_16_16_SNorm: {
		out->x = (f32)reader::readS16(offset, byteOrder) / 32767.0f;
		out->y = (f32)reader::readS16(offset + 2, byteOrder) / 32767.0f;
		out->z = (f32)reader::readS16(offset + 4, byteOrder) / 32767.0f;
		out->w = (f32)reader::readS16(offset + 6, byteOrder) / 32767.0f;
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_16_16_16_16_UInt: {
		out->x = (f32)reader::readU16(offset, byteOrder);
		out->y = (f32)reader::readU16(offset + 2, byteOrder);
		out->z = (f32)reader::readU16(offset + 4, byteOrder);
		out->w = (f32)reader::readU16(offset + 6, byteOrder);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_16_16_16_16_SInt: {
		out->x = (f32)reader::readS16(offset, byteOrder);
		out->y = (f32)reader::readS16(offset + 2, byteOrder);
		out->z = (f32)reader::readS16(offset + 4, byteOrder);
		out->w = (f32)reader::readS16(offset + 6, byteOrder);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_16_16_16_16_Single: {
		out->x = (f32)reader::readF16(offset, byteOrder);
		out->y = (f32)reader::readF16(offset + 2, byteOrder);
		out->z = (f32)reader::readF16(offset + 4, byteOrder);
		out->w = (f32)reader::readF16(offset + 6, byteOrder);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_16_16_16_16_UIntToSingle: {
		out->x = (f32)reader::readU16(offset, byteOrder);
		out->y = (f32)reader::readU16(offset + 2, byteOrder);
		out->z = (f32)reader::readU16(offset + 4, byteOrder);
		out->w = (f32)reader::readU16(offset + 6, byteOrder);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_16_16_16_16_SIntToSingle: {
		out->x = (f32)reader::readS16(offset, byteOrder);
		out->y = (f32)reader::readS16(offset + 2, byteOrder);
		out->z = (f32)reader::readS16(offset + 4, byteOrder);
		out->w = (f32)reader::readS16(offset + 6, byteOrder);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_32_32_UInt: {
		out->x = (f32)reader::readU32(offset, byteOrder);
		out->y = (f32)reader::readU32(offset + 4, byteOrder);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_32_32_SInt: {
		out->x = (f32)reader::readS32(offset, byteOrder);
		out->y = (f32)reader::readS32(offset + 4, byteOrder);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_32_32_Single: {
		out->x = reader::readF32(offset, byteOrder);
		out->y = reader::readF32(offset + 4, byteOrder);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_32_32_32_UInt: {
		out->x = (f32)reader::readU32(offset, byteOrder);
		out->y = (f32)reader::readU32(offset + 4, byteOrder);
		out->z = (f32)reader::readU32(offset + 8, byteOrder);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_32_32_32_SInt: {
		out->x = (f32)reader::readS32(offset, byteOrder);
		out->y = (f32)reader::readS32(offset + 4, byteOrder);
		out->z = (f32)reader::readS32(offset + 8, byteOrder);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_32_32_32_Single: {
		out->x = reader::readF32(offset, byteOrder);
		out->y = reader::readF32(offset + 4, byteOrder);
		out->z = reader::readF32(offset + 8, byteOrder);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_32_32_32_32_UInt: {
		out->x = (f32)reader::readU32(offset, byteOrder);
		out->y = (f32)reader::readU32(offset + 4, byteOrder);
		out->z = (f32)reader::readU32(offset + 8, byteOrder);
		out->w = (f32)reader::readU32(offset + 12, byteOrder);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_32_32_32_32_SInt: {
		out->x = (f32)reader::readS32(offset, byteOrder);
		out->y = (f32)reader::readS32(offset + 4, byteOrder);
		out->z = (f32)reader::readS32(offset + 8, byteOrder);
		out->w = (f32)reader::readS32(offset + 12, byteOrder);
		return hk::ResultSuccess();
	}

	case AttributeFormat::Format_32_32_32_32_Single: {
		out->x = reader::readF32(offset, byteOrder);
		out->y = reader::readF32(offset + 4, byteOrder);
		out->z = reader::readF32(offset + 8, byteOrder);
		out->w = reader::readF32(offset + 12, byteOrder);
		return hk::ResultSuccess();
	}

	case AttributeFormat::None:
	default: {
		fprintf(stderr, "error: unimplemented attribute format (%x)\n", (u16)fmt);
		return ResultInvalidAttributeFormat();
	}
	}
}

u32 readIdxFormat(const u8* offset, IndexFormat fmt, util::ByteOrder byteOrder) {
	switch (fmt) {
	case IndexFormat::U8: return reader::readU8(offset);
	case IndexFormat::U16: return reader::readU16(offset, byteOrder);
	case IndexFormat::U32: return reader::readU32(offset, byteOrder);
	}

	return 0;
}

} // namespace bfres
