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
	case NodeType::BinaryAlignment:;
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

hk::ValueOrResult<hk::Tuple<const u8*, const u8*>> Reader::getContainerOffsets(u32 idx) const {
	const u8* typeOffset;
	const u8* valueOffset;

	if (HK_TRY(getType()) == NodeType::Array) {
		typeOffset = mOffset + 4 + idx;
		valueOffset = mOffset + util::roundUp(4 + getSize(), 4) + 4 * idx;
	} else if (HK_TRY(getType()) == NodeType::Hash) {
		u32 hashIdx = mKeyOrder.at(idx);
		typeOffset = mOffset + 7 + 8 * hashIdx;
		valueOffset = mOffset + 8 + 8 * hashIdx;
	} else {
		return ResultWrongNodeType();
	}
	if (idx >= getSize()) return ResultOutOfBounds();

	hk::Tuple<const u8*, const u8*> out = { typeOffset, valueOffset };
	return out;
}

hk::ValueOrResult<NodeType> Reader::getTypeByIdx(u32 idx) const {
	const u8 *typeOffset, *valueOffset;
	tie(typeOffset, valueOffset) = HK_TRY(getContainerOffsets(idx));

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

hk::ValueOrResult<Reader> Reader::getContainerByIdx(u32 idx) const {
	const u8 *typeOffset, *valueOffset;
	tie(typeOffset, valueOffset) = HK_TRY(getContainerOffsets(idx));

	NodeType childType = HK_TRY(readType(typeOffset));
	if (childType != NodeType::Array && childType != NodeType::Hash) return ResultWrongNodeType();

	u32 offset = reader::readU32(valueOffset, mHeader.mByteOrder);

	Reader container;
	return HK_TRY(container.init(*this, offset));
}

hk::ValueOrResult<const u8*> Reader::getNodeByIdx(u32 idx, NodeType expectedType) const {
	const u8* valueOffset;
	NodeType childType;
	tie(valueOffset, childType) = HK_TRY(getNodeByIdxNoType(idx));

	if (childType != expectedType) return ResultWrongNodeType();

	return valueOffset;
}

hk::ValueOrResult<hk::Tuple<const u8*, NodeType>> Reader::getNodeByIdxNoType(u32 idx) const {
	const u8 *typeOffset, *valueOffset;
	tie(typeOffset, valueOffset) = HK_TRY(getContainerOffsets(idx));

	hk::Tuple<const u8*, NodeType> out = { valueOffset, HK_TRY(readType(typeOffset)) };
	return out;
}

hk::Result Reader::getStringByIdx(std::string* out, u32 idx) const {
	const u8* offset = HK_TRY(getNodeByIdx(idx, NodeType::String));

	u32 strIdx = reader::readU32(offset, mHeader.mByteOrder);
	*out = getValueString(strIdx);
	return hk::ResultSuccess();
}

hk::Result Reader::getBinaryByIdx(std::vector<u8>* outData, u32* outAlignment, u32 idx) const {
	const u8* offset;
	NodeType type;
	tie(offset, type) = HK_TRY(getNodeByIdxNoType(idx));

	if (type != NodeType::Binary && type != NodeType::BinaryAlignment) return ResultWrongNodeType();

	const u8* binaryOffset = mFileData + reader::readU32(offset, mHeader.mByteOrder);
	u32 size = reader::readU32(binaryOffset, mHeader.mByteOrder);
	if (type == NodeType::Binary) {
		*outAlignment = 0;
		*outData = reader::readBytes(binaryOffset + 4, size);
	} else {
		*outAlignment = reader::readU32(binaryOffset + 4, mHeader.mByteOrder);
		*outData = reader::readBytes(binaryOffset + 8, size);
	}
	return hk::ResultSuccess();
}

hk::ValueOrResult<bool> Reader::getBoolByIdx(u32 idx) const {
	const u8* offset = HK_TRY(getNodeByIdx(idx, NodeType::Bool));

	// note: treats non-boolean values (i.e. values that are not 0 or 1) as true
	u32 value = reader::readU32(offset, mHeader.mByteOrder);
	return value != 0;
}

hk::ValueOrResult<s32> Reader::getS32ByIdx(u32 idx) const {
	const u8* offset = HK_TRY(getNodeByIdx(idx, NodeType::S32));

	return reader::readS32(offset, mHeader.mByteOrder);
}

hk::ValueOrResult<f32> Reader::getF32ByIdx(u32 idx) const {
	const u8* offset = HK_TRY(getNodeByIdx(idx, NodeType::F32));

	return reader::readF32(offset, mHeader.mByteOrder);
}

hk::ValueOrResult<u32> Reader::getU32ByIdx(u32 idx) const {
	const u8* offset = HK_TRY(getNodeByIdx(idx, NodeType::U32));

	return reader::readU32(offset, mHeader.mByteOrder);
}

hk::ValueOrResult<s64> Reader::getS64ByIdx(u32 idx) const {
	if (mHeader.mVersion < 3) return ResultInvalidVersion();

	const u8* offset = HK_TRY(getNodeByIdx(idx, NodeType::S64));

	u32 value = reader::readU32(offset, mHeader.mByteOrder);
	return reader::readS64(mFileData + value, mHeader.mByteOrder);
}

hk::ValueOrResult<f64> Reader::getF64ByIdx(u32 idx) const {
	if (mHeader.mVersion < 3) return ResultInvalidVersion();

	const u8* offset = HK_TRY(getNodeByIdx(idx, NodeType::F64));

	u32 value = reader::readU32(offset, mHeader.mByteOrder);
	return reader::readF64(mFileData + value, mHeader.mByteOrder);
}

hk::ValueOrResult<u64> Reader::getU64ByIdx(u32 idx) const {
	if (mHeader.mVersion < 3) return ResultInvalidVersion();

	const u8* offset = HK_TRY(getNodeByIdx(idx, NodeType::U64));

	u32 value = reader::readU32(offset, mHeader.mByteOrder);
	return reader::readU64(mFileData + value, mHeader.mByteOrder);
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

hk::ValueOrResult<Reader> Reader::getContainerByKey(const std::string& key) const {
	if (HK_TRY(getType()) != NodeType::Hash) return ResultWrongNodeType();

	// TODO: implement binary search
	for (u32 i = 0; i < getSize(); i++) {
		const u8* childOffset = mOffset + 4 + i * 8;
		u32 keyIdx = reader::readU24(childOffset, mHeader.mByteOrder);
		if (util::isEqual(key, getHashString(keyIdx))) {
			NodeType type = HK_TRY(readType(childOffset + 3));
			if (type != NodeType::Array && type != NodeType::Hash) return ResultWrongNodeType();

			u32 value = reader::readU32(childOffset + 4, mHeader.mByteOrder);
			Reader container;
			return HK_TRY(container.init(*this, value));
		}
	}

	return ResultInvalidKey();
}

hk::ValueOrResult<const u8*> Reader::getNodeByKey(
	const std::string& key, NodeType expectedType
) const {
	const u8* valueOffset;
	NodeType childType;
	tie(valueOffset, childType) = HK_TRY(getNodeByKeyNoType(key));

	if (childType != expectedType) return ResultWrongNodeType();

	return valueOffset;
}

hk::ValueOrResult<hk::Tuple<const u8*, NodeType>> Reader::getNodeByKeyNoType(
	const std::string& key
) const {
	if (HK_TRY(getType()) != NodeType::Hash) return ResultWrongNodeType();

	// TODO: implement binary search
	for (u32 i = 0; i < getSize(); i++) {
		const u8* childOffset = mOffset + 4 + i * 8;
		u32 keyIdx = reader::readU24(childOffset, mHeader.mByteOrder);
		if (util::isEqual(key, getHashString(keyIdx))) {
			NodeType childType = HK_TRY(readType(childOffset + 3));

			hk::Tuple<const u8*, NodeType> out = { childOffset, childType };
			return out;
		}
	}

	return ResultInvalidKey();
}

hk::Result Reader::getStringByKey(std::string* out, const std::string& key) const {
	const u8* offset = HK_TRY(getNodeByKey(key, NodeType::String));

	u32 value = reader::readU32(offset + 4, mHeader.mByteOrder);
	*out = getValueString(value);
	return hk::ResultSuccess();
}

hk::Result Reader::getBinaryByKey(
	std::vector<u8>* outData, u32* outAlignment, std::string& key
) const {
	const u8* offset;
	NodeType type;
	tie(offset, type) = HK_TRY(getNodeByKeyNoType(key));

	if (type != NodeType::Binary && type != NodeType::BinaryAlignment) return ResultWrongNodeType();

	const u8* binaryOffset = mFileData + reader::readU32(offset, mHeader.mByteOrder);
	u32 size = reader::readU32(binaryOffset, mHeader.mByteOrder);
	if (type == NodeType::Binary) {
		*outAlignment = 0;
		*outData = reader::readBytes(binaryOffset + 4, size);
	} else {
		*outAlignment = reader::readU32(binaryOffset + 4, mHeader.mByteOrder);
		*outData = reader::readBytes(binaryOffset + 8, size);
	}
	return hk::ResultSuccess();
}

hk::ValueOrResult<bool> Reader::getBoolByKey(const std::string& key) const {
	const u8* offset = HK_TRY(getNodeByKey(key, NodeType::Bool));

	// note: treats non-boolean values (i.e. values that are not 0 or 1) as true
	u32 value = reader::readU32(offset + 4, mHeader.mByteOrder);
	return value != 0;
}

hk::ValueOrResult<s32> Reader::getS32ByKey(const std::string& key) const {
	const u8* offset = HK_TRY(getNodeByKey(key, NodeType::S32));

	return reader::readS32(offset + 4, mHeader.mByteOrder);
}

hk::ValueOrResult<f32> Reader::getF32ByKey(const std::string& key) const {
	const u8* offset = HK_TRY(getNodeByKey(key, NodeType::F32));

	return reader::readF32(offset + 4, mHeader.mByteOrder);
}

hk::ValueOrResult<u32> Reader::getU32ByKey(const std::string& key) const {
	const u8* offset = HK_TRY(getNodeByKey(key, NodeType::U32));

	return reader::readU32(offset + 4, mHeader.mByteOrder);
}

hk::ValueOrResult<s64> Reader::getS64ByKey(const std::string& key) const {
	if (mHeader.mVersion < 3) return ResultInvalidVersion();

	const u8* offset = HK_TRY(getNodeByKey(key, NodeType::S64));

	u32 value = reader::readU32(offset + 4, mHeader.mByteOrder);
	return reader::readS64(mFileData + value, mHeader.mByteOrder);
}

hk::ValueOrResult<f64> Reader::getF64ByKey(const std::string& key) const {
	if (mHeader.mVersion < 3) return ResultInvalidVersion();

	const u8* offset = HK_TRY(getNodeByKey(key, NodeType::F64));

	u32 value = reader::readU32(offset + 4, mHeader.mByteOrder);
	return reader::readF64(mFileData + value, mHeader.mByteOrder);
}

hk::ValueOrResult<u64> Reader::getU64ByKey(const std::string& key) const {
	if (mHeader.mVersion < 3) return ResultInvalidVersion();

	const u8* offset = HK_TRY(getNodeByKey(key, NodeType::U64));

	u32 value = reader::readU32(offset + 4, mHeader.mByteOrder);
	return reader::readU64(mFileData + value, mHeader.mByteOrder);
}

} // namespace byml
