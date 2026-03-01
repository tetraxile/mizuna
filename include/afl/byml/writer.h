#pragma once

#include <array>
#include <set>
#include <string>
#include <vector>

#include "afl/byml/common.h"
#include "afl/util.h"

namespace byml {

class Writer {
private:
	struct Node {
		Node(NodeType type) : mType(type) {}

		bool isContainerNode() const { return mType == NodeType::Array || mType == NodeType::Hash; }

		bool isValueNode() const {
			return mType == NodeType::Bool || mType == NodeType::S32 || mType == NodeType::F32 ||
			       mType == NodeType::U32 || mType == NodeType::Null;
		}

		bool isValue64Node() const {
			return mType == NodeType::S64 || mType == NodeType::F64 || mType == NodeType::U64;
		}

		virtual ~Node() {}

		virtual u32 calcSize() const = 0;
		virtual void write(std::vector<u8>& outputBuffer, u32 offset) const = 0;

		NodeType mType;
	};

	struct StringTable {
		void addString(const std::string& string);
		void write(std::vector<u8>& outputBuffer, u32 offset) const;
		u32 find(const std::string& string) const;

		u32 size() const { return mStrings.size(); }

		bool isEmpty() const { return size() == 0; }

		u32 calcSize() const {
			if (mStrings.empty()) return 0;

			u32 headerSize = 8 + 4 * size();
			u32 stringsSize = 0;
			for (const auto& str : mStrings)
				stringsSize += str.size() + 1;
			return headerSize + util::roundUp(stringsSize, 4);
		}

		std::set<std::string> mStrings;
	};

	struct Container : Node {
		Container(NodeType type) : Node(type) {}

		virtual size_t size() const = 0;
		virtual void writeContainer(std::vector<u8>& outputBuffer) const = 0;

		void write(std::vector<u8>& outputBuffer, u32 offset) const override {
			writer::writeU32LE(outputBuffer, offset, mOffset);
		}

		void setOffset(u32 offset) { mOffset = offset; }

		u32 mOffset;
	};

	struct Array : Container {
		Array() : Container(NodeType::Array) {}

		void writeContainer(std::vector<u8>& outputBuffer) const override;

		size_t size() const override { return mNodes.size(); }

		u32 calcSize() const override {
			u32 listTypesSize = util::roundUp(mNodes.size(), 4);
			return 4 + listTypesSize + mNodes.size() * 4;
		}

		std::vector<Node*> mNodes;
	};

	struct Hash : Container {
		Hash(const StringTable& hashKeyStringTable) :
			Container(NodeType::Hash), mHashKeyStringTable(hashKeyStringTable) {}

		void writeContainer(std::vector<u8>& outputBuffer) const override;

		size_t size() const override { return mNodes.size(); }

		u32 calcSize() const override { return 4 + 8 * mNodes.size(); }

		const StringTable& mHashKeyStringTable;
		std::vector<std::pair<std::string, Node*>> mNodes;
	};

	struct ValueNode : Node {
		ValueNode(NodeType type) : Node(type) {}

		u32 calcSize() const override { return 4; }
	};

	struct String : ValueNode {
		String(const std::string& value, const StringTable& valueStringTable) :
			ValueNode(NodeType::String), mValueStringTable(valueStringTable), mValue(value) {}

		void write(std::vector<u8>& outputBuffer, u32 offset) const override {
			u32 index = mValueStringTable.find(mValue);
			writer::writeU32LE(outputBuffer, offset, index);
		}

		const StringTable& mValueStringTable;
		const std::string mValue;
	};

	struct Bool : ValueNode {
		Bool(bool value) : ValueNode(NodeType::Bool), mValue(value) {}

		void write(std::vector<u8>& outputBuffer, u32 offset) const override {
			writer::writeU32LE(outputBuffer, offset, mValue);
		}

		bool mValue;
	};

	struct S32 : ValueNode {
		S32(s32 value) : ValueNode(NodeType::S32), mValue(value) {}

		void write(std::vector<u8>& outputBuffer, u32 offset) const override {
			writer::writeS32LE(outputBuffer, offset, mValue);
		}

		s32 mValue;
	};

	struct F32 : ValueNode {
		F32(f32 value) : ValueNode(NodeType::F32), mValue(value) {}

		void write(std::vector<u8>& outputBuffer, u32 offset) const override {
			writer::writeF32LE(outputBuffer, offset, mValue);
		}

		f32 mValue;
	};

	struct U32 : ValueNode {
		U32(u32 value) : ValueNode(NodeType::U32), mValue(value) {}

		void write(std::vector<u8>& outputBuffer, u32 offset) const override {
			writer::writeU32LE(outputBuffer, offset, mValue);
		}

		u32 mValue;
	};

	struct Null : ValueNode {
		Null() : ValueNode(NodeType::Null) {}

		void write(std::vector<u8>& outputBuffer, u32 offset) const override {
			writer::writeU32LE(outputBuffer, offset, 0);
		}
	};

	struct Value64Node : Node {
		Value64Node(NodeType type) : Node(type) {}

		virtual void writeData64(std::vector<u8>& outputBuffer, u32 offset) const = 0;

		u32 calcSize() const override { return 4; }

		void write(std::vector<u8>& outputBuffer, u32 offset) const override {
			writer::writeU32LE(outputBuffer, offset, mOffset);
		}

		void setOffset(u32 offset) { mOffset = offset; }

		u32 mOffset;
	};

	struct S64 : Value64Node {
		S64(s64 value) : Value64Node(NodeType::S64), mValue(value) {}

		void writeData64(std::vector<u8>& outputBuffer, u32 offset) const override {
			writer::writeS64LE(outputBuffer, offset, mValue);
		}

		s64 mValue;
	};

	struct U64 : Value64Node {
		U64(u64 value) : Value64Node(NodeType::U64), mValue(value) {}

		void writeData64(std::vector<u8>& outputBuffer, u32 offset) const override {
			writer::writeU64LE(outputBuffer, offset, mValue);
		}

		u64 mValue;
	};

	struct F64 : Value64Node {
		F64(f64 value) : Value64Node(NodeType::F64), mValue(value) {}

		void writeData64(std::vector<u8>& outputBuffer, u32 offset) const override {
			writer::writeF64LE(outputBuffer, offset, mValue);
		}

		f64 mValue;
	};

public:
	Writer(u32 version) : mVersion(version) {}

	void saveToVec(std::vector<u8>& out, util::ByteOrder byteOrder = util::ByteOrder::Little);
	void save(const std::string& filename, util::ByteOrder byteOrder = util::ByteOrder::Little);

	hk::Result pushArray();
	hk::Result pushHash();
	hk::Result pushArray(const std::string& key);
	hk::Result pushHash(const std::string& key);
	hk::Result pop();

	hk::Result addString(const std::string& value);
	hk::Result addBool(bool value);
	hk::Result addS32(s32 value);
	hk::Result addF32(f32 value);
	hk::Result addU32(u32 value);
	hk::Result addS64(s64 value);
	hk::Result addU64(u64 value);
	hk::Result addF64(f64 value);
	hk::Result addNull();

	hk::Result addString(const std::string& key, const std::string& value);
	hk::Result addBool(const std::string& key, bool value);
	hk::Result addS32(const std::string& key, s32 value);
	hk::Result addF32(const std::string& key, f32 value);
	hk::Result addU32(const std::string& key, u32 value);
	hk::Result addS64(const std::string& key, s64 value);
	hk::Result addU64(const std::string& key, u64 value);
	hk::Result addF64(const std::string& key, f64 value);
	hk::Result addNull(const std::string& key);

private:
	hk::Result addNode(Node* node);
	hk::Result addNode(const std::string& key, Node* node);

	constexpr static u32 STACK_SIZE = 16;

	const u32 mVersion;
	s32 mStackIdx = -1;
	std::array<Container*, STACK_SIZE> mContainerStack;
	std::vector<Container*> mContainerList;
	StringTable mHashKeyStringTable;
	StringTable mValueStringTable;
	std::vector<Value64Node*> mData64;
};

} // namespace byml
