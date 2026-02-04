#pragma once
#include <cstdio>

namespace amemory::detail
{

    template <typename Value>
    struct compare
    {

        template <typename OnMach>
        void operator()(const Value& value, void* buffer, std::size_t size, OnMach&& mach) const noexcept
        {
            auto* raw = static_cast<std::byte*>(buffer);
            for (std::size_t off = 0; off + sizeof(Value) <= size; off += sizeof(Value))
            {
                Value v;
                std::memcpy(&v, raw + off, sizeof(Value));

                if (v == value)
                    mach(reinterpret_cast<Value*>(raw + off));
            }
        }
    };
}
