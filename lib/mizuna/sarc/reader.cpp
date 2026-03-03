#include "mizuna/sarc/reader.h"

#include <filesystem>

#include "mizuna/results.h"

namespace sarc {

hk::Result Reader::init() {
	HK_TRY(initHeader(&mContents[0]));

	HK_TRY(readSFAT(&mContents[0x14]));

	HK_TRY(readSFNT(&mContents[0x14 + 0xc + 0x10 * mFiles.size()]));

	return hk::ResultSuccess();
}

hk::Result Reader::initHeader(const u8* offset) {
	HK_TRY(reader::checkSignature(offset, "SARC", 4));

	u16 headerSize = reader::readU16(offset + 4, util::ByteOrder::Little);
	HK_TRY(reader::readByteOrder(&mHeader.mByteOrder, offset + 6, 0xFEFF));

	mHeader.mFileSize = reader::readU32(offset + 8, mHeader.mByteOrder);
	mHeader.mDataOffset = reader::readU32(offset + 0xc, mHeader.mByteOrder);
	mHeader.mVersion = reader::readU16(offset + 0x10, mHeader.mByteOrder);
	// 2 padding bytes

	if (mHeader.mVersion != 0x100) {
		u8 major = mHeader.mVersion;
		u8 minor = mHeader.mVersion >> 8;

		fprintf(
			stderr, "error: unsupported SARC version (got: %d.%d, expected: 1.0)\n", major, minor
		);
		return ResultUnimplementedVersion();
	}

	return hk::ResultSuccess();
}

hk::Result Reader::readSFAT(const u8* offset) {
	HK_TRY(reader::checkSignature(offset, "SFAT", 4));
	u16 headerSize = reader::readU16(offset + 4, mHeader.mByteOrder);
	if (headerSize != 0xc) return ResultHeaderSizeMismatch();
	u16 nodeCount = reader::readU16(offset + 6, mHeader.mByteOrder);
	u32 hashKey = reader::readU32(offset + 8, mHeader.mByteOrder);

	mFiles.reserve(nodeCount);
	for (s32 i = 0; i < nodeCount; i++) {
		const u8* fileOffset = offset + 0xc + 0x10 * i;
		File file;
		file.mHash = reader::readU32(fileOffset, mHeader.mByteOrder);
		file.mAttrs = reader::readU32(fileOffset + 4, mHeader.mByteOrder);
		file.mStartOffset = reader::readU32(fileOffset + 8, mHeader.mByteOrder);
		file.mEndOffset = reader::readU32(fileOffset + 0xc, mHeader.mByteOrder);
		mFiles.push_back(file);
	}

	return hk::ResultSuccess();
}

hk::Result Reader::readSFNT(const u8* offset) {
	HK_TRY(reader::checkSignature(offset, "SFNT", 4));
	u16 headerSize = reader::readU16(offset + 4, mHeader.mByteOrder);
	if (headerSize != 0x8) return ResultHeaderSizeMismatch();

	const u8* nameTableOffset = offset + 8;
	for (File& file : mFiles) {
		if (file.mAttrs & (1 << 24)) {
			u16 nameOffset = (file.mAttrs & 0xffff) * 4;
			file.mName = reader::readString(nameTableOffset + nameOffset);
		}
	}

	return hk::ResultSuccess();
}

const std::set<std::string> Reader::getFilenames() {
	std::set<std::string> filenames;
	for (const File& file : mFiles)
		filenames.insert(file.mName);

	return filenames;
}

hk::Result Reader::saveFile(const std::string& outDir, const std::string& filename) {
	fs::path basePath(outDir);
	fs::create_directory(basePath);

	for (const File& file : mFiles) {
		if (!util::isEqual(file.mName, filename)) continue;

		fs::path filePath = basePath / file.mName;
		fs::create_directories(filePath.parent_path().c_str());

		const u8* offset = &mContents[0] + mHeader.mDataOffset + file.mStartOffset;
		u32 size = file.mEndOffset - file.mStartOffset;
		std::vector<u8> contents = reader::readBytes(offset, size);
		util::writeFile(basePath / file.mName, contents);
		return hk::ResultSuccess();
	}

	return ResultFileNotFound();
}

hk::Result Reader::saveAll(const std::string& outDir) {
	fs::path basePath(outDir);
	fs::create_directory(basePath);

	for (const File& file : mFiles) {
		fs::path filePath = basePath / file.mName;
		fs::create_directories(filePath.parent_path().c_str());

		const u8* offset = &mContents[0] + mHeader.mDataOffset + file.mStartOffset;
		u32 size = file.mEndOffset - file.mStartOffset;
		std::vector<u8> contents = reader::readBytes(offset, size);
		util::writeFile(filePath, contents);
	}

	return hk::ResultSuccess();
}

hk::Result Reader::getFileData(std::vector<u8>& out, const std::string& filename) {
	for (const File& file : mFiles) {
		if (!util::isEqual(file.mName, filename)) continue;

		const u8* offset = &mContents[0] + mHeader.mDataOffset + file.mStartOffset;
		u32 size = file.mEndOffset - file.mStartOffset;
		out = reader::readBytes(offset, size);
		return hk::ResultSuccess();
	}

	return ResultFileNotFound();
}

hk::Result Reader::getFileSize(u32* out, const std::string& filename) {
	for (const File& file : mFiles) {
		if (!util::isEqual(file.mName, filename)) continue;

		*out = file.mEndOffset - file.mStartOffset;
		return hk::ResultSuccess();
	}

	return ResultFileNotFound();
}

} // namespace sarc
