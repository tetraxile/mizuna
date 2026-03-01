#pragma once

#include <cassert>
#include <format>
#include <hk/util/Math.h>
#include <vector>

#include "afl/util.h"

namespace bfres {

class Reader;

enum class AttributeFormat : u16 {
	None = 0x0000,
	// 8 bits (8 x 1)
	Format_8_UNorm = 0x0201,
	Format_8_UInt = 0x0203,
	Format_8_SNorm = 0x0202,
	Format_8_SInt = 0x0204,
	Format_8_UIntToSingle = 0x0208,
	Format_8_SIntToSingle = 0x020A,
	// 8 bits (4 x 2)
	Format_4_4_UNorm = 0x0100,
	// 16 bits (16 x 1)
	Format_16_UNorm = 0x0A01,
	Format_16_UInt = 0x0A02,
	Format_16_SNorm = 0x0A03,
	Format_16_SInt = 0x0A04,
	Format_16_Single = 0x0A05,
	Format_16_UIntToSingle = 0x0308,
	Format_16_SIntToSingle = 0x030A,
	// 16 bits (8 x 2)
	Format_8_8_UNorm = 0x0901,
	Format_8_8_UInt = 0x0903,
	Format_8_8_SNorm = 0x0902,
	Format_8_8_SInt = 0x0904,
	Format_8_8_UIntToSingle = 0x0408,
	Format_8_8_SIntToSingle = 0x040A,
	// 32 bits (16 x 2)
	Format_16_16_UNorm = 0x1201,
	Format_16_16_SNorm = 0x1202,
	Format_16_16_UInt = 0x1203,
	Format_16_16_SInt = 0x1204,
	Format_16_16_Single = 0x1205,
	Format_16_16_UIntToSingle = 0x0708,
	Format_16_16_SIntToSingle = 0x070A,
	// 32 bits (10/11 x 3)
	Format_10_11_11_Single = 0x0908,
	// 32 bits (8 x 4)
	Format_8_8_8_8_UNorm = 0x0B01,
	Format_8_8_8_8_SNorm = 0x0B02,
	Format_8_8_8_8_UInt = 0x0B03,
	Format_8_8_8_8_SInt = 0x0B04,
	Format_8_8_8_8_UIntToSingle = 0x0B08,
	Format_8_8_8_8_SIntToSingle = 0x0B0A,
	// 32 bits (10 x 3 + 2)
	Format_10_10_10_2_UNorm = 0x0B00,
	Format_10_10_10_2_UInt = 0x0B09,
	Format_10_10_10_2_SNorm = 0x0E02,
	Format_10_10_10_2_SInt = 0x9B09,
	// 64 bits (16 x 4)
	Format_16_16_16_16_UNorm = 0x1501,
	Format_16_16_16_16_SNorm = 0x1502,
	Format_16_16_16_16_UInt = 0x1503,
	Format_16_16_16_16_SInt = 0x1504,
	Format_16_16_16_16_Single = 0x1505,
	Format_16_16_16_16_UIntToSingle = 0x0E08,
	Format_16_16_16_16_SIntToSingle = 0x0E0A,
	// 32 bits (32 x 1)
	Format_32_UInt = 0x1403,
	Format_32_SInt = 0x1604,
	Format_32_Single = 0x1605,
	// 64 bits (32 x 2)
	Format_32_32_UInt = 0x1703,
	Format_32_32_SInt = 0x1704,
	Format_32_32_Single = 0x1705,
	// 96 bits (32 x 3)
	Format_32_32_32_UInt = 0x1803,
	Format_32_32_32_SInt = 0x1804,
	Format_32_32_32_Single = 0x1805,
	// 128 bits (32 x 4)
	Format_32_32_32_32_UInt = 0x1903,
	Format_32_32_32_32_SInt = 0x1904,
	Format_32_32_32_32_Single = 0x1905
};

enum class PrimitiveType : u32 {
	Points = 0,
	Lines,
	LineStrip,
	Triangles,
	TriangleStrip,
	LinesAdjacency,
	LineStripAdjacency,
	TrianglesAdjancency,
	TriangleStripAdjacency,
	Patches,
};

enum class IndexFormat : u32 {
	U8 = 0,
	U16,
	U32,
};

// TODO: try different values and see what these look like
enum class WrapMode : u8 {
	Repeat,
	Mirror,
	Clamp,
	ClampToEdge,
	MirrorOnce,
	MirrorOnceClampToEdge,
};

// TODO: try different values and see what these look like
enum class CompareFunc : u8 {
	Never,
	Less,
	Equal,
	LessEqual,
	Greater,
	NotEqual,
	GreaterEqual,
	Always,
};

hk::Result readAttrFormat(
	hk::util::Vector4f* out, const u8* offset, AttributeFormat fmt, util::ByteOrder byteOrder
);
u32 getAttrFormatStride(AttributeFormat fmt);
u32 getAttrFormatSize(AttributeFormat fmt);
u32 readIdxFormat(const u8* offset, IndexFormat fmt, util::ByteOrder byteOrder);
u32 getIndexFormatStride(IndexFormat fmt);

class DataNode {
public:
	DataNode(const Reader* file, const u8* base, const util::ByteOrder byteOrder) :
		mFile(file), mBase(base), mByteOrder(byteOrder) {}

	virtual hk::Result read(const u8* offset) = 0;
	virtual u32 size() const = 0;

	virtual std::string toString() const { return typeid(this).name(); }

	hk::Result readHeader(const u8* offset, const std::string& signature);
	std::string readString(const u8* offset);

protected:
	const Reader* mFile;
	const u8* mBase;
	const util::ByteOrder mByteOrder;
};

template <typename T>
concept IsDataNode = requires(T t) {
	t.cSize;
	requires std::derived_from<T, DataNode>;
};

template <IsDataNode T>
class Dict {
private:
	struct Node {
		const static u32 cSize = 0x10;

		Node(s32 ref, u16 left, u16 right, const std::string key, T* value) :
			reference(ref), idxLeft(left), idxRight(right), key(key), value(value) {}

		s32 reference;
		u16 idxLeft;
		u16 idxRight;
		const std::string key;
		T* value = nullptr;
	};

public:
	Dict(const Reader* file, const u8* base, const util::ByteOrder byteOrder) :
		mFile(file), mBase(base), mByteOrder(byteOrder) {}

	hk::Result read(u64 offset, u64 arrayOffset) {
		// node count excludes root node
		s32 nodeCount = reader::readS32(mBase + offset + 0x4, mByteOrder);

		const u8* nodesOffset = mBase + offset + 0x8;
		const u8* valuesOffset = mBase + arrayOffset;

		// read root node
		HK_TRY(readNode(nodesOffset, valuesOffset, true));

		for (s32 i = 0; i < nodeCount; i++) {
			HK_TRY(
				readNode(nodesOffset + (i + 1) * Node::cSize, valuesOffset + i * T::cSize, false)
			);
		}

		return hk::ResultSuccess();
	}

	void print(s32 indent = 0) const {
		std::string indentStr(indent, '\t');
		for (const Node* node : mNodes) {
			printf(
				"%s%s: %s\n", indentStr.c_str(), node->key.c_str(), node->value->toString().c_str()
			);
		}
	}

	T* getValue(s32 idx) { return mNodes.at(idx)->value; }

	size_t getNodeCount() const { return mNodes.size(); }

private:
	hk::Result readNode(const u8* nodeOffset, const u8* valueOffset, bool isRoot = false) {
		s32 reference = reader::readS32(nodeOffset, mByteOrder);
		u16 idxLeft = reader::readU16(nodeOffset + 0x4, mByteOrder);
		u16 idxRight = reader::readU16(nodeOffset + 0x6, mByteOrder);
		u64 keyOffset = reader::readU64(nodeOffset + 0x8, mByteOrder);

		u16 keyLen = reader::readU16(mBase + keyOffset, mByteOrder);
		std::string key = reader::readString(mBase + keyOffset + 2, keyLen);

		if (isRoot) {
			assert(reference == -1);
			mRootNode = new Node(reference, idxLeft, idxRight, key, nullptr);
		} else {
			T* value = new T(mFile, mBase, mByteOrder);
			HK_TRY(value->read(valueOffset));
			mNodes.push_back(new Node(reference, idxLeft, idxRight, key, value));
		}

		return hk::ResultSuccess();
	}

	const Reader* mFile;
	const u8* mBase;
	const util::ByteOrder mByteOrder;

	Node* mRootNode = nullptr;
	std::vector<Node*> mNodes;
};

struct String : public DataNode {
	String(const Reader* file, const u8* base, const util::ByteOrder byteOrder) :
		DataNode(file, base, byteOrder) {}

	hk::Result read(const u8* offset) override {
		mValue = readString(offset);
		// printf("str: %s\n", mValue.c_str());
		return hk::ResultSuccess();
	}

	std::string toString() const override { return mValue; }

	const static u32 cSize = 0x8;

	u32 size() const override { return cSize; }

	std::string mValue;
};

class BufferInfo : public DataNode {
public:
	BufferInfo(const Reader* file, const u8* base, const util::ByteOrder byteOrder) :
		DataNode(file, base, byteOrder) {}

	hk::Result read(const u8* offset) override;

	u32 getBufferSize() const { return mBufferSize; }

	u64 getBufferOffset() const { return mBufferOffset; }

	const static u32 cSize = 0x20;

	u32 size() const override { return cSize; }

private:
	u32 mBufferSize = 0;
	u64 mBufferOffset = 0;
};

struct VertexAttribute : public DataNode {
	VertexAttribute(const Reader* file, const u8* base, const util::ByteOrder byteOrder) :
		DataNode(file, base, byteOrder) {}

	hk::Result read(const u8* offset) override;

	const static u32 cSize = 0x10;

	u32 size() const override { return cSize; }

	const std::string& getName() const { return mName; }

	std::string mName; // _p0 = position, _n0 = normal, _c0 = color, _t0 = tangent, _b0 = bitangent,
	                   // _w0 = bone weight, _i0 = bone index
	AttributeFormat mFormat = AttributeFormat::None;
	u16 mBufferOffset = 0; // byte offset to attribute within each vertex
	u8 mBufferIdx = 0;     // index into FVTX's mVtxBuffers
	bool mIsDynamic = false;
};

struct VertexBuffer {
	VertexBuffer(const u8* offset, u32 count, u32 stride) :
		mOffset(offset), mCount(count), mStride(stride) {}

	const u8* mOffset;
	const u32 mCount;
	const u32 mStride;
};

class FVTX : public DataNode {
public:
	FVTX(const Reader* file, const u8* base, const util::ByteOrder byteOrder) :
		DataNode(file, base, byteOrder) {}

	hk::Result read(const u8* offset) override;

	const static u32 cSize = 0x60;

	u32 size() const override { return cSize; }

	// private:
	std::vector<VertexBuffer*> mBuffers;
	Dict<VertexAttribute>* mAttrs = nullptr;
	u64 mMemoryPoolOffset = 0;
	u16 mIndex = 0;
	u32 mVertexCount = 0;
	u32 mSkinWeightInfluence = 0;
};

struct SubMesh {
	SubMesh(const u8* offset, u32 count, IndexFormat fmt) :
		mOffset(offset), mCount(count), mIdxFormat(fmt) {}

	const static u32 cSize = 0x8;

	const u8* mOffset;
	const u32 mCount;
	const IndexFormat mIdxFormat;
};

class Mesh : public DataNode {
public:
	Mesh(const Reader* file, const u8* base, const util::ByteOrder byteOrder) :
		DataNode(file, base, byteOrder) {}

	hk::Result read(const u8* offset) override;

	const static u32 cSize = 0x38;

	u32 size() const override { return cSize; }

	// private:
	std::vector<SubMesh*> mSubMeshes;

	PrimitiveType mPrimType;
	IndexFormat mIdxFormat;
	u32 mIdxCount = 0;
	u32 mFirstVertexIdx = 0;
	u64 mMemoryPoolOffset = 0;
};

class FSHP : public DataNode {
public:
	FSHP(const Reader* file, const u8* base, const util::ByteOrder byteOrder) :
		DataNode(file, base, byteOrder) {}

	hk::Result read(const u8* offset) override;

	const static u32 cSize = 0x70;

	u32 size() const override { return cSize; }

	const std::string& getName() const { return mName; }

	Mesh* getMesh(s32 idx) const { return mMeshes.at(idx); }

	u16 getVtxBufferIdx() const { return mVtxBufferIdx; }

private:
	std::vector<Mesh*> mMeshes;
	std::string mName;
	u16 mVtxBufferIdx = 0;
};

class RenderInfo : public DataNode {
private:
	enum DataType : u8 {
		S32 = 0,
		F32 = 1,
		String = 2,
	};

	struct Data {
		Data(DataType type) : type(type) {}

		virtual ~Data() = default;

		DataType type;
	};

	struct DataS32 : public Data {
		DataS32(s32 value) : Data(S32), value(value) {}

		s32 value;
	};

	struct DataF32 : public Data {
		DataF32(f32 value) : Data(F32), value(value) {}

		f32 value;
	};

	struct DataStr : public Data {
		DataStr(std::string value) : Data(String), value(value) {}

		std::string value;
	};

public:
	RenderInfo(const Reader* file, const u8* base, const util::ByteOrder byteOrder) :
		DataNode(file, base, byteOrder) {}

	hk::Result read(const u8* offset) override;

	const static u32 cSize = 0x18;

	u32 size() const override { return cSize; }

	std::string toString() const override {
		std::string str = std::format("{} (", mName);
		for (const Data* data : mData) {
			if (data->type == S32)
				str += std::format("{}, ", static_cast<const DataS32*>(data)->value);
			else if (data->type == F32)
				str += std::format("{}, ", static_cast<const DataF32*>(data)->value);
			else if (data->type == String) {
				str += std::format("{}, ", static_cast<const DataStr*>(data)->value);
			}
		}
		str += ")";
		return str;
	}

private:
	std::string mName;
	std::vector<Data*> mData;
};

class ShaderAssign : public DataNode {
public:
	ShaderAssign(const Reader* file, const u8* base, const util::ByteOrder byteOrder) :
		DataNode(file, base, byteOrder) {}

	hk::Result read(const u8* offset) override;

	const static u32 cSize = 0x48;

	u32 size() const override { return cSize; }

	// private:
	std::string mShaderArcName;
	std::string mShadingModelName;
	u32 mRevision = 0;
	Dict<String>* mAttrAssigns = nullptr;
	Dict<String>* mSamplerAssigns = nullptr;
	Dict<String>* mShaderOptions = nullptr;
};

class Sampler : public DataNode {
public:
	Sampler(const Reader* file, const u8* base, const util::ByteOrder byteOrder) :
		DataNode(file, base, byteOrder) {}

	hk::Result read(const u8* offset) override;

	const static u32 cSize = 0x20;

	u32 size() const override { return cSize; }
};

class ShaderParam : public DataNode {
public:
	ShaderParam(const Reader* file, const u8* base, const util::ByteOrder byteOrder) :
		DataNode(file, base, byteOrder) {}

	hk::Result read(const u8* offset) override;

	const static u32 cSize = 0x20;

	u32 size() const override { return cSize; }

private:
	std::string mName;
};

class FMAT : public DataNode {
public:
	FMAT(const Reader* file, const u8* base, const util::ByteOrder byteOrder) :
		DataNode(file, base, byteOrder) {}

	hk::Result read(const u8* offset) override;

	const static u32 cSize = 0xb8;

	u32 size() const override { return cSize; }

	std::string getName() const { return mName; }

	// private:
	std::string mName;
	ShaderAssign* mShaderAssign = nullptr;
	Dict<RenderInfo>* mRenderInfos = nullptr;
	Dict<Sampler>* mSamplers = nullptr;
	Dict<ShaderParam>* mShaderParams = nullptr;
};

class FMDL : public DataNode {
public:
	FMDL(const Reader* file, const u8* base, const util::ByteOrder byteOrder) :
		DataNode(file, base, byteOrder) {}

	hk::Result read(const u8* offset) override;

	const static u32 cSize = 0x78;

	u32 size() const override { return cSize; }

	const std::string& getName() const { return mName; }

	FVTX* getVtxBuffer(s32 idx) const { return mVtxBuffers.at(idx); }

	FSHP* getShape(s32 idx) const { return mShapes ? mShapes->getValue(idx) : nullptr; }

	size_t getShapeCount() const { return mShapes ? mShapes->getNodeCount() : 0; }

	FMAT* getMaterial(s32 idx) { return mMaterials ? mMaterials->getValue(idx) : nullptr; }

	size_t getMaterialCount() const { return mMaterials ? mMaterials->getNodeCount() : 0; }

private:
	std::vector<FVTX*> mVtxBuffers;
	Dict<FSHP>* mShapes = nullptr;
	Dict<FMAT>* mMaterials = nullptr;

	std::string mName;
	std::string mPathName;
	u32 mTotalVtxCount = 0;
};

class Reader {
public:
	Reader(const std::vector<u8>& fileContents) : mContents(fileContents), mBase(&mContents[0]) {}

	hk::Result read();
	hk::Result readHeader(const u8* offset);
	hk::Result exportGLTF(const fs::path& output);

	FMDL* getModel(s32 idx) const { return mModels ? mModels->getValue(idx) : nullptr; }

	size_t getModelCount() const { return mModels ? mModels->getNodeCount() : 0; }

	u32 getGPUBufferSize() const { return mBufferInfo->getBufferSize(); }

	u64 getGPUBufferOffset() const { return mBufferInfo->getBufferOffset(); }

private:
	const std::vector<u8>& mContents;
	const u8* mBase;
	util::ByteOrder mByteOrder;

	BufferInfo* mBufferInfo = nullptr;
	Dict<FMDL>* mModels = nullptr;
};

} // namespace bfres
