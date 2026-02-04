#pragma once

#include <cstdint>
#include <string_view>

namespace amemory::meta
{
    enum class memory_t : std::int32_t
    {
        EMPTY = -1,
        UNKNOWN = 0,
        ANONYMOUS = 1 << 0,
        ASHMEM = 1 << 1,
        BAD = 1 << 2,
        CODE_APP = 1 << 3,
        CODE_SYS = 1 << 4,
        C_ALLOC = 1 << 5,
        C_BSS = 1 << 6,
        C_DATA = 1 << 7,
        C_HEAP = 1 << 8,
        JAVA = 1 << 9,
        JAVA_HEAP = 1 << 10,
        PPSSPP = 1 << 11,
        STACK = 1 << 12,
        VIDEO = 1 << 13,
        OTHER = 1 << 14
    };

    inline std::string_view to_string(memory_t memory_type) noexcept
    {
        switch (memory_type)
        {
        case memory_t::EMPTY:       return "EMPTY";
        case memory_t::UNKNOWN:     return "UNKNOWN";
        case memory_t::ANONYMOUS:   return "ANONYMOUS";
        case memory_t::ASHMEM:      return "ASHMEM";
        case memory_t::BAD:         return "BAD";
        case memory_t::CODE_APP:    return "CODE_APP";
        case memory_t::CODE_SYS:    return "CODE_SYS";
        case memory_t::C_ALLOC:     return "C_ALLOC";
        case memory_t::C_BSS:       return "C_BSS";
        case memory_t::C_DATA:      return "C_DATA";
        case memory_t::C_HEAP:      return "C_HEAP";
        case memory_t::JAVA:        return "JAVA";
        case memory_t::JAVA_HEAP:   return "JAVA_HEAP";
        case memory_t::PPSSPP:      return "PPSSPP";
        case memory_t::STACK:       return "STACK";
        case memory_t::VIDEO:       return "VIDEO";
        case memory_t::OTHER:       return "OTHER";
        default:                    return "UNKNOWN";
        }
    }
} // namespace amemory
