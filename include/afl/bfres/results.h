#pragma once

#include <hk/Result.h>

namespace bfres {

HK_RESULT_MODULE(3)
HK_DEFINE_RESULT_RANGE(Bfres, 0, 100)
HK_DEFINE_RESULT(InvalidAttributeFormat, 0)
HK_DEFINE_RESULT(InvalidVertexAttribute, 1)
HK_DEFINE_RESULT(InvalidFileExtension, 2)
HK_DEFINE_RESULT(UnsupportedVersion, 3)

} // namespace bfres
