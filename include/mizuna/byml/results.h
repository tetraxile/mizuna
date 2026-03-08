#pragma once

#include <hk/Result.h>

namespace byml {

HK_RESULT_MODULE(2)
HK_DEFINE_RESULT_RANGE(Byaml, 0, 20)
HK_DEFINE_RESULT(WrongNodeType, 0)
HK_DEFINE_RESULT(InvalidNodeType, 1)
HK_DEFINE_RESULT(InvalidKey, 2)
HK_DEFINE_RESULT(OutOfBounds, 3)
HK_DEFINE_RESULT(EmptyStack, 4)
HK_DEFINE_RESULT(FullStack, 5)
HK_DEFINE_RESULT(InvalidVersion, 6)

} // namespace byml
