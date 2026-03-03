#include "mizuna/sarc/writer.h"

#include <algorithm>
#include <span>
#include <unordered_map>

#include "mizuna/util.h"

namespace sarc {

void Writer::saveToVec(std::vector<u8>& out, util::ByteOrder byteOrder, u32 alignment) {
	if (byteOrder != util::ByteOrder::Little) {
		fprintf(stderr, "error: unimplemented big-endian sarc writer\n");
		return;
	}

	const u32 sfatStart = 0x14;
	const u32 sfatEntryStart = sfatStart + 0xc;
	const u32 sfntStart = sfatEntryStart + 0x10 * mFiles.size();
	const u32 sfntEntryStart = sfntStart + 0x8;

	u32 namesLen = 0;
	u32 filesLen = 0;
	for (const File& file : mFiles) {
		namesLen = util::roundUp(namesLen, 4) + file.mName.length() + 1;
		filesLen = util::roundUp(filesLen, alignment) + file.mData.size();
	}

	const u32 dataOffset = util::roundUp(sfntEntryStart + namesLen, alignment);
	const u32 fileSize = dataOffset + filesLen;

	// file header
	writer::writeString(out, 0x00, "SARC", false);
	writer::writeU16LE(out, 0x04, 0x14);   // header size
	writer::writeU16LE(out, 0x06, 0xfeff); // byte order mark
	// writer::writeU32LE(out, 0x08, fileSize);
	writer::writeU32LE(out, 0x0c, dataOffset);
	writer::writeU16LE(out, 0x10, mVersion);
	// 0x12 - 2 bytes padding

	// sfat header
	writer::writeString(out, sfatStart + 0x00, "SFAT", false);
	writer::writeU16LE(out, sfatStart + 0x04, 0xc); // header size
	writer::writeU16LE(out, sfatStart + 0x06, mFiles.size());
	writer::writeU32LE(out, sfatStart + 0x08, mHashMultiplier);

	// sfnt header
	writer::writeString(out, sfntStart + 0x00, "SFNT", false);
	writer::writeU16LE(out, sfntStart + 0x04, 0x8);
	// 0x06 - 2 bytes padding

	u32 sfatEntry = sfatEntryStart;
	u32 sfntEntry = sfntEntryStart;
	u32 dataEntry = dataOffset;
	u32 dataEnd = dataOffset;
	std::unordered_map<u32, u8> hashes;

	std::stable_sort(mFiles.begin(), mFiles.end(), [this](const File& i1, const File& i2) {
		return calcHash(i1.mName) < calcHash(i2.mName);
	});

	for (u32 i = 0; i < mFiles.size(); i++) {
		const File& file = mFiles[i];

		u32 filenameHash = calcHash(file.mName);
		if (hashes.contains(filenameHash)) {
			hashes[filenameHash]++;
		} else {
			hashes[filenameHash] = 1;
		}
		u8 hashCount = hashes[filenameHash];
		u32 fileAttributes = (hashCount << 24) | (((sfntEntry - sfntEntryStart) >> 2) & 0xffffff);

		writer::writeU32LE(out, sfatEntry + 0x00, filenameHash);
		writer::writeU32LE(out, sfatEntry + 0x04, fileAttributes);
		writer::writeU32LE(out, sfatEntry + 0x08, dataEntry - dataOffset);
		writer::writeU32LE(out, sfatEntry + 0x0c, dataEntry - dataOffset + file.mData.size());

		writer::writeString(out, sfntEntry, file.mName);
		writer::writeBytes(out, dataEntry, file.mData);

		sfatEntry += 0x10;
		sfntEntry = util::roundUp(sfntEntry + file.mName.length() + 1, 4);
		dataEnd = dataEntry + file.mData.size();
		dataEntry = util::roundUp(dataEntry + file.mData.size(), alignment);
	}

	// TODO: figure out why file size pre-calculation above doesn't work
	writer::writeU32LE(out, 0x08, dataEnd);
}

void Writer::save(const std::string& filename, util::ByteOrder byteOrder, u32 alignment) {
	std::vector<u8> outputBuffer;
	saveToVec(outputBuffer, byteOrder, alignment);
	util::writeFile(filename, outputBuffer);
}

void Writer::addFile(const std::string& filename, const std::vector<u8>& fileData) {
	mFiles.push_back({ filename, fileData });
}

u32 Writer::calcHash(const std::string& str) const {
	u32 hash = 0;
	const auto strBytes = std::as_bytes(std::span { str.data(), str.size() });
	for (const std::byte byte : strBytes) {
		hash = hash * mHashMultiplier + (u8)byte;
	}
	return hash;
}

} // namespace sarc
