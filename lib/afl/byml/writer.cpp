#include "afl/byml/writer.h"

#include <algorithm>
#include <cassert>

namespace byml {

void Writer::saveToVec(std::vector<u8>& out, util::ByteOrder byteOrder) {
	if (byteOrder != util::ByteOrder::Little) {
		fprintf(stderr, "error: unimplemented big-endian byml writer\n");
		return;
	}

	writer::writeU16LE(out, 0, 0x4259);   // byte order mark
	writer::writeU16LE(out, 2, mVersion); // version

	u32 hashKeyTableOffset = 0x10;
	u32 valueStringTableOffset = hashKeyTableOffset + mHashKeyStringTable.calcSize();
	u32 data64Offset = valueStringTableOffset + mValueStringTable.calcSize();
	u32 rootOffset = util::roundUp(data64Offset + mData64.size() * 8, 4);

	writer::writeU32LE(out, 0x4, mHashKeyStringTable.isEmpty() ? 0 : hashKeyTableOffset);
	writer::writeU32LE(out, 0x8, mValueStringTable.isEmpty() ? 0 : valueStringTableOffset);
	writer::writeU32LE(out, 0xc, mContainerList.empty() ? 0 : rootOffset);

	mHashKeyStringTable.write(out, hashKeyTableOffset);
	mValueStringTable.write(out, valueStringTableOffset);

	u32 writePtr = data64Offset;
	for (auto* node : mData64) {
		node->setOffset(writePtr);
		node->writeData64(out, writePtr);
		writePtr += 8;
	}

	writePtr = rootOffset;
	for (Container* container : mContainerList) {
		container->setOffset(writePtr);
		writePtr += container->calcSize();
	}

	for (Container* container : mContainerList) {
		container->writeContainer(out);
	}
}

void Writer::save(const std::string& filename, util::ByteOrder byteOrder) {
	std::vector<u8> outputBuffer;
	saveToVec(outputBuffer, byteOrder);
	util::writeFile(filename, outputBuffer);
}

hk::Result Writer::pushArray() {
	// check if stack level is max, if it is then return error
	if (mStackIdx == STACK_SIZE - 1) return ResultFullStack();

	Array* child = new Array;

	// if stack is not empty, add array to container on top of stack
	if (mStackIdx > -1) {
		Container* top = mContainerStack[mStackIdx];
		if (top->mType != NodeType::Array) return ResultWrongNodeType();
		Array* parent = static_cast<Array*>(top);
		parent->mNodes.push_back(child);
	}

	// push array to stack and add to container list
	mContainerStack[++mStackIdx] = child;
	mContainerList.push_back(child);

	return hk::ResultSuccess();
}

hk::Result Writer::pushHash() {
	// check if stack level is max, if it is then return error
	if (mStackIdx == STACK_SIZE - 1) return ResultFullStack();

	Hash* child = new Hash(mHashKeyStringTable);

	// if stack is not empty, add hash to container on top of stack
	if (mStackIdx > -1) {
		Container* top = mContainerStack[mStackIdx];
		if (top->mType != NodeType::Array) return ResultWrongNodeType();
		Array* parent = static_cast<Array*>(top);
		parent->mNodes.push_back(child);
	}

	// push hash to stack and add to container list
	mContainerStack[++mStackIdx] = child;
	mContainerList.push_back(child);

	return hk::ResultSuccess();
}

hk::Result Writer::pushArray(const std::string& key) {
	// check if stack level is invalid, if it is then return error
	if (mStackIdx == STACK_SIZE - 1) return ResultFullStack();
	if (mStackIdx == -1) return ResultEmptyStack();

	Array* child = new Array;
	mHashKeyStringTable.addString(key);

	// if stack is not empty, add array to container on top of stack
	Container* top = mContainerStack[mStackIdx];
	if (top->mType != NodeType::Hash) return ResultWrongNodeType();
	Hash* parent = static_cast<Hash*>(top);
	parent->mNodes.push_back(std::make_pair(key, child));

	// push array to stack and add to container list
	mContainerStack[++mStackIdx] = child;
	mContainerList.push_back(child);

	return hk::ResultSuccess();
}

hk::Result Writer::pushHash(const std::string& key) {
	// check if stack level is invalid, if it is then return error
	if (mStackIdx == STACK_SIZE - 1) return ResultFullStack();
	if (mStackIdx == -1) return ResultEmptyStack();

	Hash* child = new Hash(mHashKeyStringTable);
	mHashKeyStringTable.addString(key);

	// if stack is not empty, add hash to container on top of stack
	Container* top = mContainerStack[mStackIdx];
	if (top->mType != NodeType::Hash) return ResultWrongNodeType();
	Hash* parent = static_cast<Hash*>(top);
	parent->mNodes.push_back(std::make_pair(key, child));

	// push hash to stack and add to container list
	mContainerStack[++mStackIdx] = child;
	mContainerList.push_back(child);

	return hk::ResultSuccess();
}

hk::Result Writer::pop() {
	// check if stack level is -1, if it is then return error
	if (mStackIdx == -1) return ResultEmptyStack();

	// decrease stack level
	mStackIdx--;

	return hk::ResultSuccess();
}

hk::Result Writer::addNode(Node* node) {
	if (mStackIdx == -1) return ResultEmptyStack();

	Container* top = mContainerStack[mStackIdx];
	assert(top && "top of stack is not initialized");
	if (top->mType != NodeType::Array) return ResultWrongNodeType();

	Array* topArray = static_cast<Array*>(top);
	topArray->mNodes.push_back(node);
	return hk::ResultSuccess();
}

hk::Result Writer::addNode(const std::string& key, Node* node) {
	if (mStackIdx == -1) return ResultEmptyStack();

	Container* top = mContainerStack[mStackIdx];
	assert(top && "top of stack is not initialized");
	if (top->mType != NodeType::Hash) return ResultWrongNodeType();

	Hash* topHash = static_cast<Hash*>(top);
	mHashKeyStringTable.addString(key);
	topHash->mNodes.push_back(std::make_pair(key, node));
	return hk::ResultSuccess();
}

hk::Result Writer::addString(const std::string& value) {
	mValueStringTable.addString(value);
	return addNode(new String(value, mValueStringTable));
}

hk::Result Writer::addBool(bool value) {
	return addNode(new Bool(value));
}

hk::Result Writer::addS32(s32 value) {
	return addNode(new S32(value));
}

hk::Result Writer::addF32(f32 value) {
	return addNode(new F32(value));
}

hk::Result Writer::addU32(u32 value) {
	return addNode(new U32(value));
}

hk::Result Writer::addS64(s64 value) {
	S64* node = new S64(value);
	mData64.push_back(node);
	return addNode(node);
}

hk::Result Writer::addU64(u64 value) {
	U64* node = new U64(value);
	mData64.push_back(node);
	return addNode(node);
}

hk::Result Writer::addF64(f64 value) {
	F64* node = new F64(value);
	mData64.push_back(node);
	return addNode(node);
}

hk::Result Writer::addNull() {
	return addNode(new Null);
}

hk::Result Writer::addString(const std::string& key, const std::string& value) {
	mValueStringTable.addString(value);
	return addNode(key, new String(value, mValueStringTable));
}

hk::Result Writer::addBool(const std::string& key, bool value) {
	return addNode(key, new Bool(value));
}

hk::Result Writer::addS32(const std::string& key, s32 value) {
	return addNode(key, new S32(value));
}

hk::Result Writer::addF32(const std::string& key, f32 value) {
	return addNode(key, new F32(value));
}

hk::Result Writer::addU32(const std::string& key, u32 value) {
	return addNode(key, new U32(value));
}

hk::Result Writer::addS64(const std::string& key, s64 value) {
	S64* node = new S64(value);
	mData64.push_back(node);
	return addNode(key, node);
}

hk::Result Writer::addU64(const std::string& key, u64 value) {
	U64* node = new U64(value);
	mData64.push_back(node);
	return addNode(key, node);
}

hk::Result Writer::addF64(const std::string& key, f64 value) {
	F64* node = new F64(value);
	mData64.push_back(node);
	return addNode(key, node);
}

hk::Result Writer::addNull(const std::string& key) {
	return addNode(key, new Null);
}

void Writer::StringTable::addString(const std::string& string) {
	mStrings.insert(string);
}

void Writer::StringTable::write(std::vector<u8>& outputBuffer, u32 offset) const {
	if (isEmpty()) return;

	writer::writeU8(outputBuffer, offset, (u8)NodeType::StringTable);
	writer::writeU24LE(outputBuffer, offset + 1, size());

	u32 addrOffset = offset + 4;
	u32 strOffset = addrOffset + 4 * size() + 4;
	for (const std::string& string : mStrings) {
		writer::writeU32LE(outputBuffer, addrOffset, strOffset - offset);
		writer::writeString(outputBuffer, strOffset, string);
		strOffset += string.size() + 1;
		addrOffset += 4;
	}

	writer::writeU32LE(outputBuffer, offset + 4 + 4 * size(), strOffset - offset);
}

u32 Writer::StringTable::find(const std::string& string) const {
	s32 i = 0;
	for (const std::string& checkString : mStrings) {
		if (util::isEqual(string, checkString)) return i;
		i++;
	}

	assert(false && "string wasn't added to string table");

	return 0xffffff;
}

void Writer::Array::writeContainer(std::vector<u8>& outputBuffer) const {
	writer::writeU8(outputBuffer, mOffset, (u8)mType);
	writer::writeU24LE(outputBuffer, mOffset + 1, size());

	u32 typeOffset = mOffset + 4;
	for (Node* node : mNodes)
		writer::writeU8(outputBuffer, typeOffset++, (u8)node->mType);

	u32 valueOffset = mOffset + 4 + util::roundUp(size(), 4);
	for (Node* node : mNodes) {
		node->write(outputBuffer, valueOffset);
		valueOffset += 4;
	}
}

void Writer::Hash::writeContainer(std::vector<u8>& outputBuffer) const {
	writer::writeU8(outputBuffer, mOffset, (u8)mType);
	writer::writeU24LE(outputBuffer, mOffset + 1, size());

	std::vector<std::pair<u32, Node*>> idxNodes;
	idxNodes.reserve(mNodes.size());
	for (const auto& [key, node] : mNodes) {
		idxNodes.push_back(std::make_pair(mHashKeyStringTable.find(key), node));
	}

	std::sort(idxNodes.begin(), idxNodes.end(), [](const auto& i1, const auto& i2) {
		return i1.first < i2.first;
	});

	u32 nodeOffset = mOffset + 4;
	for (const auto& [keyIdx, node] : idxNodes) {
		writer::writeU24LE(outputBuffer, nodeOffset, keyIdx);
		writer::writeU8(outputBuffer, nodeOffset + 3, (u8)node->mType);
		node->write(outputBuffer, nodeOffset + 4);
		nodeOffset += 8;
	}
}

} // namespace byml
