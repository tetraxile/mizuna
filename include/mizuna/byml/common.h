#pragma once

#include <hk/types.h>

#include "results.h"

namespace byml {

enum class NodeType : u8 {
	String = 0xa0,
	Array = 0xc0,
	Hash = 0xc1,
	StringTable = 0xc2,
	Bool = 0xd0,
	S32 = 0xd1,
	F32 = 0xd2,
	U32 = 0xd3,
	S64 = 0xd4,
	U64 = 0xd5,
	F64 = 0xd6,
	Null = 0xff,
};

} // namespace byml
