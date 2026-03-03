// Yaz0 compression file format
// explanation of format in include/yaz0.h

#include "mizuna/yaz0.h"

#include <algorithm>

#include "mizuna/results.h"
#include "mizuna/util.h"

namespace yaz0 {

hk::Result decompress(std::vector<u8>& output, const std::vector<u8>& input) {
	u8 magic[4] = { input[0], input[1], input[2], input[3] };
	if (magic[0] != 'Y' || magic[1] != 'a' || magic[2] != 'z' || magic[3] != '0') {
		return ResultBadSignature();
	}

	u32 uncompressedSize = reader::readU32(&input[4], util::ByteOrder::Big);
	u32 alignment = reader::readU32(&input[8], util::ByteOrder::Big);

	output.resize(uncompressedSize);

	u32 source = 0x10;
	u32 dest = 0;

	while (dest < uncompressedSize) {
		u8 codeByte = input.at(source++);
		for (s8 i = 7; i >= 0; i--) {
			if (dest >= uncompressedSize) break;

			bool isCopy = (codeByte >> i) & 0x01;
			if (isCopy) {
				output.at(dest++) = input.at(source++);
			} else {
				u16 data = reader::readU16(&input[source], util::ByteOrder::Big);
				source += 2;

				u16 count = data >> 0xc;
				if (count == 0)
					count = input.at(source++) + 0x12;
				else
					count += 2;

				u16 offset = (data & 0xfff) + 1;

				for (s32 j = 0; j < count; j++) {
					output.at(dest) = output.at(dest - offset);
					dest++;
				}
			}
		}
	}

	return hk::ResultSuccess();
}

void compress(std::vector<u8>& output, const std::vector<u8>& input, u32 alignment) {
	u32 uncompressedSize = input.size();

	output.resize(0x10);
	writer::writeU32(output, 0x0, 0x59617a30, util::ByteOrder::Big);
	writer::writeU32(output, 0x4, uncompressedSize, util::ByteOrder::Big);
	writer::writeU32(output, 0x8, alignment, util::ByteOrder::Big);

	// TODO: remove candidates list and just keep track of first candidate,
	// recomputing only when necessary
	std::vector<u32> candidates;
	candidates.reserve(0x1000);

	std::vector<u8> chunk(24);

	u32 readPtr = 0;
	u32 chunkDest = 0;
	u32 bufferStart = 0;

	while (readPtr < uncompressedSize) {
		u8 codeByte = 0;
		for (s32 i = 7; i >= 0; i--) {
			std::fill(candidates.begin(), candidates.end(), 0);

			// add matching bytes from buffer to candidates list
			for (u32 j = bufferStart; j < readPtr; j++) {
				if (input.at(j) == input.at(readPtr)) candidates.push_back(j);
			}

			// find which candidate matches the most input bytes (max of 0x111 bytes)
			s32 count;
			u32 savedCandidate;
			for (count = 0; count < 0x111; count++) {
				savedCandidate = candidates.front();
				if (candidates.empty() || (readPtr + count >= input.size())) break;

				for (auto it = candidates.begin(); it != candidates.end();) {
					if (input.at(readPtr + count) != input.at(*it + count))
						it = candidates.erase(it);
					else
						++it;
				}
			}

			count--;
			if (count < 3) {
				// copy a single byte to output buffer
				chunk.at(chunkDest++) = input.at(readPtr);
				count = 1;
				codeByte |= 1 << i;
			} else {
				// write compressed data to output buffer
				u16 offset = readPtr - savedCandidate - 1;

				if (count < 0x12) {
					// 2-byte compressed data
					u16 data = (count - 2) << 0xc | (offset & 0xfff);
					writer::writeU16(chunk, chunkDest, data, util::ByteOrder::Big);
					chunkDest += 2;
				} else {
					// 3-byte compressed data
					u16 data = offset & 0xfff;
					writer::writeU16(chunk, chunkDest, data, util::ByteOrder::Big);
					chunkDest += 2;
					chunk.at(chunkDest++) = count - 0x12;
				}
			}

			readPtr += count;
			bufferStart = std::max(0u, readPtr - 0x1000);

			if (readPtr >= input.size()) break;
		}

		// write code byte and then chunk to output
		output.push_back(codeByte);
		for (u32 i = 0; i < chunkDest; i++) {
			output.push_back(chunk.at(i));
		}
		chunkDest = 0;
	}
}

} // namespace yaz0
