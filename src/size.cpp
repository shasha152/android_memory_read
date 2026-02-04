//
// Created by 30980 on 2025/12/24.
//

#include <cstdio>
#include <unistd.h>

namespace amemory::detail
{
    inline std::size_t page_size()
    {
        static std::size_t ps = ::sysconf(_SC_PAGESIZE);
        return ps;
    }
}

