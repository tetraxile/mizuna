#pragma once

#include <hk/Result.h>

HK_RESULT_MODULE(1)
HK_DEFINE_RESULT_RANGE(Mizuna, 0, 20)
HK_DEFINE_RESULT(BadSignature, 0)
HK_DEFINE_RESULT(BadByteOrder, 1)
HK_DEFINE_RESULT(FileError, 2)
HK_DEFINE_RESULT(FileNotFound, 3)
HK_DEFINE_RESULT(DirNotFound, 4)
HK_DEFINE_RESULT(HeaderSizeMismatch, 5)
HK_DEFINE_RESULT(UnimplementedVersion, 6)
