#pragma once

#include <set>

#include "mizuna/util.h"

namespace sarc {

class Reader {
public:
	struct Header {
		util::ByteOrder mByteOrder;
		u32 mFileSize;
		u32 mDataOffset;
		u16 mVersion;
	};

	struct File {
		u32 mHash;
		u32 mAttrs;
		u32 mStartOffset;
		u32 mEndOffset;
		std::string mName;
	};

	Reader(const std::vector<u8>& fileContents) : mContents(fileContents) {}

	hk::Result init();
	hk::Result initHeader(const u8* offset);
	hk::Result readSFAT(const u8* offset);
	hk::Result readSFNT(const u8* offset);
	const std::set<std::string> getFilenames();
	hk::Result saveFile(const std::string& outDir, const std::string& filename);
	hk::Result saveAll(const std::string& outDir);
	hk::Result getFileData(std::vector<u8>& out, const std::string& filename);
	hk::Result getFileSize(u32* out, const std::string& filename);

private:
	const std::vector<u8>& mContents;
	Header mHeader;
	std::vector<File> mFiles;
};

} // namespace sarc
