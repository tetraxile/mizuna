#include "mizuna/bfres/reader.h"

namespace bfres {

hk::Result FSHP::read(const u8* offset) {
	HK_TRY(readHeader(offset, "FSHP"));

	mName = readString(offset + 0x10);
	u64 vtxBufferOffset = reader::readU64(offset + 0x18, mByteOrder);
	u64 meshArrayOffset = reader::readU64(offset + 0x20, mByteOrder);
	u64 skinBoneIdxArrayOffset = reader::readU64(offset + 0x28, mByteOrder);
	u64 keyShapeArrayOffset = reader::readU64(offset + 0x30, mByteOrder);
	u64 keyShapeDictOffset = reader::readU64(offset + 0x38, mByteOrder);
	u64 boundBoxArrayOffset = reader::readU64(offset + 0x40, mByteOrder);
	u64 radiusArrayOffset = reader::readU64(offset + 0x48, mByteOrder);
	u64 userDataOffset = reader::readU64(offset + 0x50, mByteOrder);
	u32 flags = reader::readU32(offset + 0x58, mByteOrder);
	u16 idx = reader::readU16(offset + 0x5c, mByteOrder);
	u16 matIdx = reader::readU16(offset + 0x5e, mByteOrder);
	u16 boneIdx = reader::readU16(offset + 0x60, mByteOrder);
	mVtxBufferIdx = reader::readU16(offset + 0x62, mByteOrder);
	u16 skinBoneIdx = reader::readU16(offset + 0x64, mByteOrder);
	u8 vtxSkinCount = reader::readU8(offset + 0x66);
	u8 meshCount = reader::readU8(offset + 0x67);
	u8 keyShapeCount = reader::readU8(offset + 0x68);
	u8 targetAttrCount = reader::readU8(offset + 0x69);

	printf("\t\tname: %s\n", mName.c_str());

	printf("\t\tvtxBufferOffset: %lx\n", vtxBufferOffset);
	printf("\t\tskinBoneIdxArrayOffset: %lx\n", skinBoneIdxArrayOffset);
	printf("\t\tkeyShapeArrayOffset: %lx\n", keyShapeArrayOffset);
	printf("\t\tkeyShapeDictOffset: %lx\n", keyShapeDictOffset);
	printf("\t\tboundBoxArrayOffset: %lx\n", boundBoxArrayOffset);
	printf("\t\tradiusArrayOffset: %lx\n", radiusArrayOffset);
	printf("\t\tuserDataOffset: %lx\n", userDataOffset);
	printf("\t\tflags: %x\n", flags);
	printf("\t\tidx: %d\n", idx);
	printf("\t\tmatIdx: %d\n", matIdx);
	printf("\t\tboneIdx: %d\n", boneIdx);
	printf("\t\tvtxBufferIdx: %d\n", mVtxBufferIdx);
	printf("\t\tskinBoneIdx: %d\n", skinBoneIdx);
	printf("\t\tvtxSkinCount: %d\n", vtxSkinCount);
	printf("\t\tkeyShapeCount: %d\n", keyShapeCount);
	printf("\t\ttargetAttrCount: %d\n", targetAttrCount);
	printf("\n");

	for (s32 i = 0; i < meshCount; i++) {
		const u8* meshOffset = mBase + meshArrayOffset + i * Mesh::cSize;
		Mesh* mesh = new Mesh(mFile, mBase, mByteOrder);
		HK_TRY(mesh->read(meshOffset));
		mMeshes.push_back(mesh);
	}

	return hk::ResultSuccess();
}

hk::Result Mesh::read(const u8* offset) {
	u64 subMeshArrayOffset = reader::readU64(offset, mByteOrder);
	mMemoryPoolOffset = reader::readU64(offset + 0x8, mByteOrder);
	u64 bufferOffset = reader::readU64(offset + 0x10, mByteOrder);
	u64 bufferSizeOffset = reader::readU64(offset + 0x18, mByteOrder);
	u32 idxBufferOffset = reader::readU32(offset + 0x20, mByteOrder);
	mPrimType = PrimitiveType(reader::readU32(offset + 0x24, mByteOrder));
	mIdxFormat = IndexFormat(reader::readU32(offset + 0x28, mByteOrder));
	mIdxCount = reader::readU32(offset + 0x2c, mByteOrder);
	mFirstVertexIdx = reader::readU32(offset + 0x30, mByteOrder);
	u16 subMeshCount = reader::readU16(offset + 0x34, mByteOrder);

	u32 stride = getIndexFormatStride(mIdxFormat);

	const u8* idxBuffer = mBase + mFile->getGPUBufferOffset() + idxBufferOffset;
	for (u16 i = 0; i < subMeshCount; i++) {
		u32 bufferSize = reader::readU32(mBase + bufferSizeOffset + i * 0x8, mByteOrder);
		u32 elemCount = bufferSize / stride;
		mSubMeshes.push_back(new SubMesh(idxBuffer, elemCount, mIdxFormat));
		idxBuffer += bufferSize;
	}

	return hk::ResultSuccess();
}

} // namespace bfres
