#pragma once

#include "mizuna/util.h"

namespace sarc {

class Writer {
public:
	struct File {
		// File(File&) = default;
		// File(File&&) = default;

		// File& operator=(File& rhs) { mName = rhs.mName; };

		std::string mName;
		std::vector<u8> mData;
	};

	Writer(u16 version = 0x100) : mVersion(version) {}

	void saveToVec(
		std::vector<u8>& out, util::ByteOrder byteOrder = util::ByteOrder::Little,
		u32 alignment = 0x80
	);
	void save(
		const std::string& filename, util::ByteOrder byteOrder = util::ByteOrder::Little,
		u32 alignment = 0x80
	);

	void addFile(const std::string& filename, const std::vector<u8>& fileData);

private:
	u32 calcHash(const std::string& str) const;

	const u16 mVersion;
	const u32 mHashMultiplier = 101;

	std::vector<File> mFiles;
};

} // namespace sarc
