#include "mizuna/bntx.h"

#include <cstdio>

hk::Result BNTX::read() {
	HK_TRY(readHeader(&mContents[0]));

	return hk::ResultSuccess();
}

hk::Result BNTX::readHeader(const u8* offset) {
	HK_TRY(reader::checkSignature(offset, "BNTX\0\0\0\0", 8));
	u32 version = reader::readU32(offset + 8, util::ByteOrder::Big);
	HK_TRY(reader::readByteOrder(&mByteOrder, offset + 0xc, 0xFEFF));
	u8 alignment = reader::readU8(offset + 0xe);
	u8 targetAddrSize = reader::readU8(offset + 0xf);
	u32 filenameOffset = reader::readU32(offset + 0x10, mByteOrder);
	u16 isRelocated = reader::readU16(offset + 0x14, mByteOrder);
	u16 firstBlockOffset = reader::readU16(offset + 0x16, mByteOrder);
	u32 relocTableOffset = reader::readU32(offset + 0x18, mByteOrder);
	u32 fileSize = reader::readU32(offset + 0x1c, mByteOrder);

	return hk::ResultSuccess();
}
