#pragma once

#include <algorithm>

namespace amemory::detail
{
    struct memory_splitter
    {
        template <typename Values, typename Fn>
        std::size_t operator()(const Values& values, std::size_t chunk_size, Fn&& fn) const
        {
            auto [start, end] = values.value;
            if (start >= end)
                return 0;

            std::size_t count = 0;
            for (auto cur = start; cur < end; )
            {
                std::size_t remain = static_cast<std::size_t>(end - cur);
                std::size_t size = std::min(remain, chunk_size);

                std::forward<Fn>(fn)(cur, size);
                cur += size;
                ++count;
            }
            return count;
        }
    };
}