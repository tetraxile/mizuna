#pragma once

#include <vector>

#include "afl/util.h"

class BNTX {
public:
	BNTX(const std::vector<u8>& fileContents) : mContents(fileContents) {}

	hk::Result read();
	hk::Result readHeader(const u8* offset);

private:
	const std::vector<u8>& mContents;
	util::ByteOrder mByteOrder;
};
