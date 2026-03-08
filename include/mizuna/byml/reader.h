#pragma once

#include <hk/ValueOrResult.h>

#include "hk/util/Tuple.h"
#include "mizuna/byml/common.h"
#include "mizuna/util.h"

namespace byml {

class Reader {
public:
	Reader();
	hk::Result init(const u8* fileData, const size fileSize);
	hk::Result init(const Reader& other, const u32 offset);

	u16 getVersion() const { return mHeader.mVersion; }

	util::ByteOrder getByteOrder() const { return mHeader.mByteOrder; }

	const std::string getHashString(u32 idx) const;
	const std::string getValueString(u32 idx) const;
	bool isExistHashString(const std::string& str) const;
	bool isExistStringValue(const std::string& str) const;

	hk::ValueOrResult<NodeType> getType() const;
	u32 getSize() const;
	bool hasKey(const std::string& key) const;

	hk::ValueOrResult<NodeType> getTypeByIdx(u32 idx) const;
	hk::ValueOrResult<NodeType> getTypeByKey(const std::string& key) const;
	hk::Result getKeyByIdx(std::string* out, u32 idx) const;

	hk::ValueOrResult<Reader> getContainerByIdx(u32 idx) const;
	hk::Result getStringByIdx(std::string* out, u32 idx) const;
	hk::Result getBinaryByIdx(std::vector<u8>* out, u32 idx) const;
	hk::ValueOrResult<bool> getBoolByIdx(u32 idx) const;
	hk::ValueOrResult<s32> getS32ByIdx(u32 idx) const;
	hk::ValueOrResult<f32> getF32ByIdx(u32 idx) const;
	hk::ValueOrResult<u32> getU32ByIdx(u32 idx) const;
	hk::ValueOrResult<s64> getS64ByIdx(u32 idx) const;
	hk::ValueOrResult<f64> getF64ByIdx(u32 idx) const;
	hk::ValueOrResult<u64> getU64ByIdx(u32 idx) const;

	hk::ValueOrResult<Reader> getContainerByKey(const std::string& key) const;
	hk::Result getStringByKey(std::string* out, const std::string& key) const;
	hk::Result getBinaryByKey(std::vector<u8>* out, const std::string& key) const;
	hk::ValueOrResult<bool> getBoolByKey(const std::string& key) const;
	hk::ValueOrResult<s32> getS32ByKey(const std::string& key) const;
	hk::ValueOrResult<f32> getF32ByKey(const std::string& key) const;
	hk::ValueOrResult<u32> getU32ByKey(const std::string& key) const;
	hk::ValueOrResult<s64> getS64ByKey(const std::string& key) const;
	hk::ValueOrResult<f64> getF64ByKey(const std::string& key) const;
	hk::ValueOrResult<u64> getU64ByKey(const std::string& key) const;

private:
	struct Header {
		util::ByteOrder mByteOrder;
		u16 mVersion;
		u32 mHashKeyTableOffset = 0;
		u32 mStringValueTableOffset = 0;
		u32 mRootOffset = 0;

		u32 mHashKeyTableSize = 0;
		u32 mStringValueTableSize = 0;
	};

	hk::Result initHeader();
	hk::Result initKeyOrder();

	hk::ValueOrResult<const u8*> getNodeByKey(const std::string& key, NodeType expectedType) const;
	hk::ValueOrResult<const u8*> getNodeByIdx(u32 idx, NodeType expectedType) const;
	hk::ValueOrResult<hk::Tuple<const u8*, NodeType>> getNodeByIdxNoType(u32 idx) const;
	hk::ValueOrResult<hk::Tuple<const u8*, NodeType>> getNodeByKeyNoType(
		const std::string& key
	) const;
	hk::ValueOrResult<hk::Tuple<const u8*, const u8*>> getContainerOffsets(u32 idx) const;
	hk::ValueOrResult<NodeType> readType(const u8* offset) const;
	hk::ValueOrResult<u32> getKeyIdx(const std::string& key) const;

	const u8* mFileData = nullptr;
	const u8* mOffset = nullptr;
	Header mHeader;
	size mFileSize = 0;

	std::vector<u32> mKeyOrder;
};

} // namespace byml
