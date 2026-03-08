#include "mizuna/byml/reader.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <numeric>

#include "mizuna/byml/results.h"
#include "mizuna/results.h"

namespace byml {

Reader::Reader() {}

hk::Result Reader::initHeader() {
	HK_TRY(reader::readByteOrder(&mHeader.mByteOrder, mFileData, 0x4259));

	mHeader.mVersion = reader::readU16(mFileData + 2, mHeader.mByteOrder);
	if (mHeader.mVersion < 2 || mHeader.mVersion > 4) {
		fprintf(
			stderr, "error: unsupported BYML version (got: %d, expected: 2, 3, 4)\n",
			mHeader.mVersion
		);
		return ResultUnimplementedVersion();
	}

	mHeader.mHashKeyTableOffset = reader::readU32(mFileData + 4, mHeader.mByteOrder);
	mHeader.mStringValueTableOffset = reader::readU32(mFileData + 8, mHeader.mByteOrder);
	mHeader.mRootOffset = reader::readU32(mFileData + 0xc, mHeader.mByteOrder);

	if (mHeader.mHashKeyTableOffset)
		mHeader.mHashKeyTableSize =
			reader::readU24(mFileData + mHeader.mHashKeyTableOffset + 1, mHeader.mByteOrder);

	if (mHeader.mStringValueTableOffset)
		mHeader.mStringValueTableSize =
			reader::readU24(mFileData + mHeader.mStringValueTableOffset + 1, mHeader.mByteOrder);

	return hk::ResultSuccess();
}

hk::Result Reader::init(const u8* fileData, const size fileSize) {
	mFileData = fileData;
	mFileSize = fileSize;

	HK_TRY(initHeader());

	if (mHeader.mRootOffset) {
		mOffset = mFileData + mHeader.mRootOffset;
		HK_TRY(initKeyOrder());
	}

	return hk::ResultSuccess();
}

hk::Result Reader::init(const Reader& other, const u32 offset) {
	mHeader = other.mHeader;
	mFileData = other.mFileData;
	mFileSize = other.mFileSize;
	mOffset = mFileData + offset;

	HK_TRY(initKeyOrder());

	return hk::ResultSuccess();
}

hk::Result Reader::initKeyOrder() {
	if (HK_TRY(getType()) != NodeType::Hash) return hk::ResultSuccess();

	std::vector<u32> offsets;
	offsets.reserve(getSize());
	for (u32 i = 0; i < getSize(); i++) {
		NodeType type = HK_TRY(readType(mOffset + 4 + i * 8 + 3));
		if (type == NodeType::Array || type == NodeType::Hash) {
			u32 offset = reader::readU32(mOffset + 8 + i * 8, mHeader.mByteOrder);
			offsets.push_back(offset);
		} else {
			offsets.push_back(0xffffffff);
		}
	}

	mKeyOrder.resize(getSize());
	std::iota(mKeyOrder.begin(), mKeyOrder.end(), 0);

	std::stable_sort(mKeyOrder.begin(), mKeyOrder.end(), [&offsets](size_t i1, size_t i2) {
		return offsets.at(i1) < offsets.at(i2);
	});

	return hk::ResultSuccess();
}

hk::ValueOrResult<NodeType> Reader::getType() const {
	return readType(mOffset);
}

hk::ValueOrResult<NodeType> Reader::readType(const u8* offset) const {
	assert(mFileData <= offset && offset < mFileData + mFileSize);

	NodeType type = (NodeType)reader::readU8(offset);
	switch (type) {
	case NodeType::String:
	case NodeType::Binary:
	case NodeType::Array:
	case NodeType::Hash:
	case NodeType::StringTable:
	case NodeType::Bool:
	case NodeType::S32:
	case NodeType::F32:
	case NodeType::U32:
	case NodeType::S64:
	case NodeType::U64:
	case NodeType::F64:
	case NodeType::Null: return type;
	}

	fprintf(
		stderr, "error: invalid node type %#02x (offset: %#lx)\n", u8(type), offset - mFileData
	);
	return ResultInvalidNodeType();
}

u32 Reader::getSize() const {
	if (mOffset == nullptr || HK_TRY(getType()) == NodeType::Null) return hk::ResultSuccess();

	return reader::readU24(mOffset + 1, mHeader.mByteOrder);
}

const std::string Reader::getHashString(u32 idx) const {
	if (idx >= mHeader.mHashKeyTableSize) return "(null)";

	const u8* base = mFileData + mHeader.mHashKeyTableOffset;
	const u8* strOffset = base + reader::readU32(base + 4 + (idx * 4), mHeader.mByteOrder);
	return reader::readString(strOffset);
}

const std::string Reader::getValueString(u32 idx) const {
	if (idx >= mHeader.mStringValueTableSize) return "(null)";

	const u8* base = mFileData + mHeader.mStringValueTableOffset;
	const u8* strOffset = base + reader::readU32(base + 4 + (idx * 4), mHeader.mByteOrder);
	return reader::readString(strOffset);
}

bool Reader::hasKey(const std::string& key) const {
	if (HK_TRY(getType()) != NodeType::Hash) return false;

	// TODO: implement binary search
	for (u32 i = 0; i < getSize(); i++) {
		const u8* childOffset = mOffset + 4 + i * 8;
		u32 keyIdx = reader::readU24(childOffset, mHeader.mByteOrder);
		if (util::isEqual(key, getHashString(keyIdx))) return true;
	}

	return false;
}

bool Reader::isExistHashString(const std::string& str) const {
	// TODO: implement binary search
	for (u32 i = 0; i < mHeader.mHashKeyTableSize; i++)
		if (util::isEqual(getHashString(i), str)) return true;

	return false;
}

bool Reader::isExistStringValue(const std::string& str) const {
	// TODO: implement binary search
	for (u32 i = 0; i < mHeader.mStringValueTableSize; i++)
		if (util::isEqual(getValueString(i), str)) return true;

	return false;
}

hk::Result Reader::getContainerOffsets(
	const u8** typeOffset, const u8** valueOffset, u32 idx
) const {
	if (HK_TRY(getType()) == NodeType::Array) {
		*typeOffset = mOffset + 4 + idx;
		*valueOffset = mOffset + util::roundUp(4 + getSize(), 4) + 4 * idx;
	} else if (HK_TRY(getType()) == NodeType::Hash) {
		u32 hashIdx = mKeyOrder.at(idx);
		*typeOffset = mOffset + 7 + 8 * hashIdx;
		*valueOffset = mOffset + 8 + 8 * hashIdx;
	} else {
		return ResultWrongNodeType();
	}
	if (idx >= getSize()) return ResultOutOfBounds();

	return hk::ResultSuccess();
}

hk::ValueOrResult<NodeType> Reader::getTypeByIdx(u32 idx) const {
	const u8 *typeOffset, *valueOffset;
	HK_TRY(getContainerOffsets(&typeOffset, &valueOffset, idx));

	return readType(typeOffset);
}

hk::Result Reader::getKeyByIdx(std::string* out, u32 idx) const {
	if (HK_TRY(getType()) != NodeType::Hash) return ResultWrongNodeType();
	if (idx >= getSize()) return hk::ResultOutOfRange();

	u32 keyIdx = mKeyOrder.at(idx);
	u32 keyValue = reader::readU24(mOffset + 4 + keyIdx * 8, mHeader.mByteOrder);

	*out = getHashString(keyValue);

	return hk::ResultSuccess();
}

hk::Result Reader::getContainerByIdx(Reader* container, u32 idx) const {
	const u8 *typeOffset, *valueOffset;
	HK_TRY(getContainerOffsets(&typeOffset, &valueOffset, idx));

	NodeType childType = HK_TRY(readType(typeOffset));
	if (childType != NodeType::Array && childType != NodeType::Hash) return ResultWrongNodeType();

	u32 value = reader::readU32(valueOffset, mHeader.mByteOrder);
	HK_TRY(container->init(*this, value));
	return hk::ResultSuccess();
}

hk::Result Reader::getNodeByIdx(const u8** offset, u32 idx, NodeType expectedType) const {
	const u8 *typeOffset, *valueOffset;
	HK_TRY(getContainerOffsets(&typeOffset, &valueOffset, idx));

	NodeType childType = HK_TRY(readType(typeOffset));
	if (childType != expectedType) return ResultWrongNodeType();

	*offset = valueOffset;
	return hk::ResultSuccess();
}

hk::Result Reader::getStringByIdx(std::string* out, u32 idx) const {
	const u8* offset;
	HK_TRY(getNodeByIdx(&offset, idx, NodeType::String));

	u32 strIdx = reader::readU32(offset, mHeader.mByteOrder);
	*out = getValueString(strIdx);
	return hk::ResultSuccess();
}

hk::Result Reader::getBinaryByIdx(std::vector<u8>* out, u32 idx) const {
	const u8* offset;
	HK_TRY(getNodeByIdx(&offset, idx, NodeType::Binary));

	const u8* binaryOffset = mFileData + reader::readU32(offset, mHeader.mByteOrder);
	u32 size = reader::readU32(binaryOffset, mHeader.mByteOrder);
	*out = reader::readBytes(binaryOffset + 4, size);
	return hk::ResultSuccess();
}

hk::Result Reader::getBoolByIdx(bool* out, u32 idx) const {
	const u8* offset;
	HK_TRY(getNodeByIdx(&offset, idx, NodeType::Bool));

	// note: treats non-boolean values (i.e. values that are not 0 or 1) as true
	u32 value = reader::readU32(offset, mHeader.mByteOrder);
	*out = value != 0;
	return hk::ResultSuccess();
}

hk::Result Reader::getS32ByIdx(s32* out, u32 idx) const {
	const u8* offset;
	HK_TRY(getNodeByIdx(&offset, idx, NodeType::S32));

	*out = reader::readS32(offset, mHeader.mByteOrder);
	return hk::ResultSuccess();
}

hk::Result Reader::getF32ByIdx(f32* out, u32 idx) const {
	const u8* offset;
	HK_TRY(getNodeByIdx(&offset, idx, NodeType::F32));

	*out = reader::readF32(offset, mHeader.mByteOrder);
	return hk::ResultSuccess();
}

hk::Result Reader::getU32ByIdx(u32* out, u32 idx) const {
	const u8* offset;
	HK_TRY(getNodeByIdx(&offset, idx, NodeType::U32));

	*out = reader::readU32(offset, mHeader.mByteOrder);
	return hk::ResultSuccess();
}

hk::Result Reader::getS64ByIdx(s64* out, u32 idx) const {
	if (mHeader.mVersion < 3) return ResultInvalidVersion();

	const u8* offset;
	HK_TRY(getNodeByIdx(&offset, idx, NodeType::S64));

	u32 value = reader::readU32(offset, mHeader.mByteOrder);
	*out = reader::readS64(mFileData + value, mHeader.mByteOrder);
	return hk::ResultSuccess();
}

hk::Result Reader::getF64ByIdx(f64* out, u32 idx) const {
	if (mHeader.mVersion < 3) return ResultInvalidVersion();

	const u8* offset;
	HK_TRY(getNodeByIdx(&offset, idx, NodeType::F64));

	u32 value = reader::readU32(offset, mHeader.mByteOrder);
	*out = reader::readF64(mFileData + value, mHeader.mByteOrder);
	return hk::ResultSuccess();
}

hk::Result Reader::getU64ByIdx(u64* out, u32 idx) const {
	if (mHeader.mVersion < 3) return ResultInvalidVersion();

	const u8* offset;
	HK_TRY(getNodeByIdx(&offset, idx, NodeType::U64));

	u32 value = reader::readU32(offset, mHeader.mByteOrder);
	*out = reader::readU64(mFileData + value, mHeader.mByteOrder);
	return hk::ResultSuccess();
}

hk::ValueOrResult<NodeType> Reader::getTypeByKey(const std::string& key) const {
	if (HK_TRY(getType()) != NodeType::Hash) return ResultWrongNodeType();

	// TODO: implement binary search
	for (u32 i = 0; i < getSize(); i++) {
		const u8* childOffset = mOffset + 4 + i * 8;
		u32 keyIdx = reader::readU24(childOffset, mHeader.mByteOrder);
		if (util::isEqual(key, getHashString(keyIdx))) {
			return readType(childOffset + 3);
		}
	}

	return ResultInvalidKey();
}

hk::Result Reader::getContainerByKey(Reader* container, const std::string& key) const {
	if (HK_TRY(getType()) != NodeType::Hash) return ResultWrongNodeType();

	// TODO: implement binary search
	for (u32 i = 0; i < getSize(); i++) {
		const u8* childOffset = mOffset + 4 + i * 8;
		u32 keyIdx = reader::readU24(childOffset, mHeader.mByteOrder);
		if (util::isEqual(key, getHashString(keyIdx))) {
			NodeType type = HK_TRY(readType(childOffset + 3));
			if (type != NodeType::Array && type != NodeType::Hash) return ResultWrongNodeType();

			u32 value = reader::readU32(childOffset + 4, mHeader.mByteOrder);
			HK_TRY(container->init(*this, value));

			return hk::ResultSuccess();
		}
	}

	return ResultInvalidKey();
}

hk::Result Reader::getNodeByKey(
	const u8** offset, const std::string& key, NodeType expectedType
) const {
	if (HK_TRY(getType()) != NodeType::Hash) return ResultWrongNodeType();

	// TODO: implement binary search
	for (u32 i = 0; i < getSize(); i++) {
		const u8* childOffset = mOffset + 4 + i * 8;
		u32 keyIdx = reader::readU24(childOffset, mHeader.mByteOrder);
		if (util::isEqual(key, getHashString(keyIdx))) {
			NodeType childType = HK_TRY(readType(childOffset + 3));
			if (childType != expectedType) return ResultWrongNodeType();

			*offset = childOffset;
			return hk::ResultSuccess();
		}
	}

	return ResultInvalidKey();
}

hk::Result Reader::getStringByKey(std::string* out, const std::string& key) const {
	const u8* offset;
	HK_TRY(getNodeByKey(&offset, key, NodeType::String));

	u32 value = reader::readU32(offset + 4, mHeader.mByteOrder);
	*out = getValueString(value);
	return hk::ResultSuccess();
}

hk::Result Reader::getBinaryByKey(std::vector<u8>* out, const std::string& key) const {
	const u8* offset;
	HK_TRY(getNodeByKey(&offset, key, NodeType::Binary));

	const u8* binaryOffset = mFileData + reader::readU32(offset, mHeader.mByteOrder);
	u32 size = reader::readU32(binaryOffset, mHeader.mByteOrder);
	*out = reader::readBytes(binaryOffset + 4, size);
	return hk::ResultSuccess();
}

hk::Result Reader::getBoolByKey(bool* out, const std::string& key) const {
	const u8* offset;
	HK_TRY(getNodeByKey(&offset, key, NodeType::Bool));

	// note: treats non-boolean values (i.e. values that are not 0 or 1) as true
	u32 value = reader::readU32(offset + 4, mHeader.mByteOrder);
	*out = value != 0;
	return hk::ResultSuccess();
}

hk::Result Reader::getS32ByKey(s32* out, const std::string& key) const {
	const u8* offset;
	HK_TRY(getNodeByKey(&offset, key, NodeType::S32));

	*out = reader::readS32(offset + 4, mHeader.mByteOrder);
	return hk::ResultSuccess();
}

hk::Result Reader::getF32ByKey(f32* out, const std::string& key) const {
	const u8* offset;
	HK_TRY(getNodeByKey(&offset, key, NodeType::F32));

	*out = reader::readF32(offset + 4, mHeader.mByteOrder);
	return hk::ResultSuccess();
}

hk::Result Reader::getU32ByKey(u32* out, const std::string& key) const {
	const u8* offset;
	HK_TRY(getNodeByKey(&offset, key, NodeType::U32));

	*out = reader::readU32(offset + 4, mHeader.mByteOrder);
	return hk::ResultSuccess();
}

hk::Result Reader::getS64ByKey(s64* out, const std::string& key) const {
	if (mHeader.mVersion < 3) return ResultInvalidVersion();

	const u8* offset;
	HK_TRY(getNodeByKey(&offset, key, NodeType::S64));

	u32 value = reader::readU32(offset + 4, mHeader.mByteOrder);
	*out = reader::readS64(mFileData + value, mHeader.mByteOrder);
	return hk::ResultSuccess();
}

hk::Result Reader::getF64ByKey(f64* out, const std::string& key) const {
	if (mHeader.mVersion < 3) return ResultInvalidVersion();

	const u8* offset;
	HK_TRY(getNodeByKey(&offset, key, NodeType::F64));

	u32 value = reader::readU32(offset + 4, mHeader.mByteOrder);
	*out = reader::readF64(mFileData + value, mHeader.mByteOrder);
	return hk::ResultSuccess();
}

hk::Result Reader::getU64ByKey(u64* out, const std::string& key) const {
	if (mHeader.mVersion < 3) return ResultInvalidVersion();

	const u8* offset;
	HK_TRY(getNodeByKey(&offset, key, NodeType::U64));

	u32 value = reader::readU32(offset + 4, mHeader.mByteOrder);
	*out = reader::readU64(mFileData + value, mHeader.mByteOrder);
	return hk::ResultSuccess();
}

} // namespace byml
