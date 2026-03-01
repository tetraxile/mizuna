#include "afl/bfres/reader.h"

namespace bfres {

hk::Result FMAT::read(const u8* offset) {
	HK_TRY(readHeader(offset, "FMAT"));

	mName = readString(offset + 0x10);
	u64 renderInfoArrayOffset = reader::readU64(offset + 0x18, mByteOrder);
	u64 renderInfoDictOffset = reader::readU64(offset + 0x20, mByteOrder);
	u64 shaderAssignOffset = reader::readU64(offset + 0x28, mByteOrder);
	u64 textureArrayOffset = reader::readU64(offset + 0x30, mByteOrder);
	u64 textureNameArrayOffset = reader::readU64(offset + 0x38, mByteOrder);
	u64 samplerArrayOffset = reader::readU64(offset + 0x40, mByteOrder);
	u64 samplerArrayOffset1 = reader::readU64(offset + 0x48, mByteOrder);
	u64 samplerDictOffset = reader::readU64(offset + 0x50, mByteOrder);
	u64 shaderParamArrayOffset = reader::readU64(offset + 0x58, mByteOrder);
	u64 shaderParamDictOffset = reader::readU64(offset + 0x60, mByteOrder);
	u64 sourceParamDataOffset = reader::readU64(offset + 0x68, mByteOrder);
	u64 userDataArrayOffset = reader::readU64(offset + 0x70, mByteOrder);
	u64 userDataDictOffset = reader::readU64(offset + 0x78, mByteOrder);
	u64 volatileFlagsOffset = reader::readU64(offset + 0x80, mByteOrder);
	u64 userPointer = reader::readU64(offset + 0x88, mByteOrder);
	u64 samplerSlotArrayOffset = reader::readU64(offset + 0x90, mByteOrder);
	u64 textureSlotArrayOffset = reader::readU64(offset + 0x98, mByteOrder);
	u32 flags = reader::readU32(offset + 0xa0, mByteOrder);
	u16 index = reader::readU16(offset + 0xa4, mByteOrder);
	u16 renderInfoCount = reader::readU16(offset + 0xa4, mByteOrder);
	u8 textureCount = reader::readU8(offset + 0xa8);
	u8 samplerCount = reader::readU8(offset + 0xa9);
	u16 shaderParamCount = reader::readU16(offset + 0xaa, mByteOrder);
	u16 shaderParamVolatileCount = reader::readU16(offset + 0xac, mByteOrder);
	u16 sourceParamSize = reader::readU16(offset + 0xae, mByteOrder);
	u16 rawParamSize = reader::readU16(offset + 0xb0, mByteOrder);
	u16 userDataCount = reader::readU16(offset + 0xb2, mByteOrder);

	printf("\t\tname: %s\n", mName.c_str());

	printf("\t\tshader assign offset: %lx\n", shaderAssignOffset);
	mShaderAssign = new ShaderAssign(mFile, mBase, mByteOrder);
	HK_TRY(mShaderAssign->read(mBase + shaderAssignOffset));

	printf("\t\trender info: %d\n", renderInfoCount);
	if (renderInfoDictOffset) {
		mRenderInfos = new Dict<RenderInfo>(mFile, mBase, mByteOrder);
		HK_TRY(mRenderInfos->read(renderInfoDictOffset, renderInfoArrayOffset));
		mRenderInfos->print(3);
	}

	printf("\t\tsamplers: %d\n", samplerCount);
	printf("\t\tsampler slot array offset: %lx\n", samplerSlotArrayOffset);
	if (samplerCount) {
		mSamplers = new Dict<Sampler>(mFile, mBase, mByteOrder);
		HK_TRY(mSamplers->read(samplerDictOffset, samplerArrayOffset1));
	}

	printf("\t\tshader params: %d\n", shaderParamCount);
	printf("\t\tvolatile flags: %lx %d\n", volatileFlagsOffset, shaderParamVolatileCount);
	if (shaderParamCount) {
		mShaderParams = new Dict<ShaderParam>(mFile, mBase, mByteOrder);
		HK_TRY(mShaderParams->read(shaderParamDictOffset, shaderParamArrayOffset));
	}

	printf("\t\ttextureArrayOffset: %lx\n", textureArrayOffset);
	printf("\t\ttextureCount: %d\n", textureCount);
	printf("\t\ttextureNameArrayOffset: %lx\n", textureNameArrayOffset);
	printf("\t\ttextureSlotArrayOffset: %lx\n", textureSlotArrayOffset);

	printf("\t\tsamplerArrayOffset: %lx\n", samplerArrayOffset);

	printf("\t\tsourceParamDataOffset: %lx\n", sourceParamDataOffset);
	printf("\t\tsourceParamSize: %x\n", sourceParamSize);
	printf("\t\tflags: %x\n", flags);
	printf("\t\tindex: %d\n", index);
	printf("\t\trawParamSize: %x\n", rawParamSize);

	printf("\t\tuserDataArrayOffset: %lx\n", userDataArrayOffset);
	printf("\t\tuserDataDictOffset: %lx\n", userDataDictOffset);
	printf("\t\tuserPointer: %lx\n", userPointer);
	printf("\t\tuserDataCount: %d\n", userDataCount);

	return hk::ResultSuccess();
}

hk::Result ShaderAssign::read(const u8* offset) {
	mShaderArcName = readString(offset);
	mShadingModelName = readString(offset + 0x8);
	u64 attrAssignsArrayOffset = reader::readU64(offset + 0x10, mByteOrder);
	u64 attrAssignsDictOffset = reader::readU64(offset + 0x18, mByteOrder);
	u64 samplerAssignsArrayOffset = reader::readU64(offset + 0x20, mByteOrder);
	u64 samplerAssignsDictOffset = reader::readU64(offset + 0x28, mByteOrder);
	u64 shaderOptionsArrayOffset = reader::readU64(offset + 0x30, mByteOrder);
	u64 shaderOptionsDictOffset = reader::readU64(offset + 0x38, mByteOrder);
	mRevision = reader::readU32(offset + 0x40, mByteOrder);
	u8 attrAssignsCount = reader::readU8(offset + 0x44);
	u8 samplerAssignsCount = reader::readU8(offset + 0x45);
	u16 shaderOptionsCount = reader::readU16(offset + 0x46, mByteOrder);

	printf("\t\t\tshaderArcName: %s\n", mShaderArcName.c_str());
	printf("\t\t\tshadingModelName: %s\n", mShadingModelName.c_str());
	printf("\t\t\trevision: %x\n", mRevision);

	printf("\t\t\tattr assigns: %d\n", attrAssignsCount);
	if (attrAssignsCount) {
		mAttrAssigns = new Dict<String>(mFile, mBase, mByteOrder);
		mAttrAssigns->read(attrAssignsDictOffset, attrAssignsArrayOffset);
		// mAttrAssigns->print(4);
	}

	printf("\t\t\tsampler assigns: %d\n", samplerAssignsCount);
	if (samplerAssignsCount) {
		mSamplerAssigns = new Dict<String>(mFile, mBase, mByteOrder);
		mSamplerAssigns->read(samplerAssignsDictOffset, samplerAssignsArrayOffset);
		// mSamplerAssigns->print(4);
	}

	printf("\t\t\tshader options: %d\n", shaderOptionsCount);
	if (shaderOptionsCount) {
		mShaderOptions = new Dict<String>(mFile, mBase, mByteOrder);
		mShaderOptions->read(shaderOptionsDictOffset, shaderOptionsArrayOffset);
		// mShaderOptions->print(4);
	}

	return hk::ResultSuccess();
}

hk::Result RenderInfo::read(const u8* offset) {
	mName = readString(offset);
	u64 dataOffset = reader::readU64(offset + 0x8, mByteOrder);
	u16 dataCount = reader::readU16(offset + 0x10, mByteOrder);
	u8 renderInfoType = reader::readU8(offset + 0x12);

	const u8* start = mBase + dataOffset;
	for (s32 i = 0; i < dataCount; i++) {
		if (renderInfoType == S32)
			mData.push_back(new DataS32(reader::readS32(start + i * sizeof(s32), mByteOrder)));
		else if (renderInfoType == F32)
			mData.push_back(new DataF32(reader::readF32(start + i * sizeof(f32), mByteOrder)));
		else if (renderInfoType == String)
			mData.push_back(new DataStr(readString(start + i * String::cSize)));
	}

	return hk::ResultSuccess();
}

hk::Result Sampler::read(const u8* offset) {
	WrapMode wrapModeU = WrapMode(reader::readU8(offset));
	WrapMode wrapModeV = WrapMode(reader::readU8(offset + 1));
	WrapMode wrapModeW = WrapMode(reader::readU8(offset + 2));
	CompareFunc compareFunc = CompareFunc(reader::readU8(offset + 3));
	u8 borderColorType = reader::readU8(offset + 4);
	u8 anisotropic = reader::readU8(offset + 5);
	u16 filterFlags = reader::readU16(offset + 6, mByteOrder);
	f32 minLOD = reader::readF32(offset + 0x8, mByteOrder);
	f32 maxLOD = reader::readF32(offset + 0xc, mByteOrder);
	f32 lodBias = reader::readF32(offset + 0x10, mByteOrder);

	return hk::ResultSuccess();
}

hk::Result ShaderParam::read(const u8* offset) {
	u64 callbackOffset = reader::readU64(offset, mByteOrder);
	mName = readString(offset + 8);
	u8 dataType = reader::readU8(offset + 0x10);
	u8 dataSize = reader::readU8(offset + 0x11);
	u16 dataOffset = reader::readU16(offset + 0x12, mByteOrder);
	s32 uniformOffset = reader::readS32(offset + 0x14, mByteOrder);
	u16 dependedIndex = reader::readU16(offset + 0x18, mByteOrder);
	u16 dependIndex = reader::readU16(offset + 0x1a, mByteOrder);

	printf("name: %s\n", mName.c_str());
	printf("callbackOffset: %lx\n", callbackOffset);
	printf("dataType: %d\n", dataType);
	printf("dataSize: %d\n", dataSize);
	printf("dataOffset: %x\n", dataOffset);
	printf("uniformOffset: %x\n", uniformOffset);
	printf("dependedIndex: %d\n", dependedIndex);
	printf("dependIndex: %d\n", dependIndex);

	return hk::ResultSuccess();
}

} // namespace bfres
