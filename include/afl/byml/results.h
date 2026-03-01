#pragma once

#include <hk/Result.h>

namespace byml {

HK_RESULT_MODULE(2)
HK_DEFINE_RESULT_RANGE(Byaml, 0, 20)
HK_DEFINE_RESULT(WrongNodeType, 0)
HK_DEFINE_RESULT(InvalidKey, 1)
HK_DEFINE_RESULT(OutOfBounds, 2)
HK_DEFINE_RESULT(EmptyStack, 3)
HK_DEFINE_RESULT(FullStack, 4)
HK_DEFINE_RESULT(InvalidVersion, 5)

} // namespace byml
