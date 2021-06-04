#pragma once
#include <cstdint>


#define GET_FLAG(p) static_cast<bool>((reinterpret_cast<uint64_t>(p) & 0x0001000000000000) >> 48)
#define GET_POINTER(type, p) reinterpret_cast<type>(reinterpret_cast<uint64_t>(p) & 0x0000ffffffffffff)
#define SET_FLAG(type, p, flag) p = reinterpret_cast<type>(reinterpret_cast<uint64_t>(GET_POINTER(type, p)) | (static_cast<uint64_t>(flag) << 48))
