#include "afl/bfres/reader.h"

namespace bfres {

hk::Result FMDL::read(const u8* offset) {
	HK_TRY(readHeader(offset, "FMDL"));

	mName = readString(offset + 0x10);
	mPathName = readString(offset + 0x18);
	u64 sklOffset = reader::readU64(offset + 0x20, mByteOrder);
	u64 vtxBufOffset = reader::readU64(offset + 0x28, mByteOrder);
	u64 shpArrayOffset = reader::readU64(offset + 0x30, mByteOrder);
	u64 shpDictOffset = reader::readU64(offset + 0x38, mByteOrder);
	u64 matArrayOffset = reader::readU64(offset + 0x40, mByteOrder);
	u64 matDictOffset = reader::readU64(offset + 0x48, mByteOrder);
	u64 userDataOffset = reader::readU64(offset + 0x50, mByteOrder);
	u16 vtxBufCount = reader::readU16(offset + 0x68, mByteOrder);
	u16 shpCount = reader::readU16(offset + 0x6a, mByteOrder);
	u16 matCount = reader::readU16(offset + 0x6c, mByteOrder);
	u16 userDataCount = reader::readU16(offset + 0x6e, mByteOrder);
	mTotalVtxCount = reader::readU32(offset + 0x70, mByteOrder);

	printf("\tname: %s\n", mName.c_str());
	printf("\n");

	if (sklOffset) {
		printf("\tskeleton offset: %lx\n", sklOffset);
		printf("\n");
	}

	if (vtxBufCount) {
		printf("\tvertex buffers: %d (%#lx)\n", vtxBufCount, vtxBufOffset);
		printf("\tentries:\n");
		for (u16 i = 0; i < vtxBufCount; i++) {
			FVTX* vtxBuffer = new FVTX(mFile, mBase, mByteOrder);
			HK_TRY(vtxBuffer->read(mBase + vtxBufOffset + i * FVTX::cSize));
			mVtxBuffers.push_back(vtxBuffer);
		}
		printf("\n");
	}

	if (shpCount) {
		printf("\tshapes: %d (%#lx, %#lx)\n", shpCount, shpArrayOffset, shpDictOffset);
		printf("\tentries:\n");
		mShapes = new Dict<FSHP>(mFile, mBase, mByteOrder);
		HK_TRY(mShapes->read(shpDictOffset, shpArrayOffset));
		printf("\n");
	}

	if (matCount) {
		printf("\tmaterials: %d (%#lx, %#lx)\n", matCount, matArrayOffset, matDictOffset);
		printf("\tentries:\n");
		mMaterials = new Dict<FMAT>(mFile, mBase, mByteOrder);
		HK_TRY(mMaterials->read(matDictOffset, matArrayOffset));
		printf("\n");
	}

	return hk::ResultSuccess();
}

} // namespace bfres
