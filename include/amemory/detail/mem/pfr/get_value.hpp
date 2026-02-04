#pragma once

#include "amemory/detail/pfr/value.hpp"
#include "amemory/detail/pfr/pointer.hpp"

namespace amemory::detail
{
    template <typename T>
    T& get_value(T& v) noexcept
    {
        return v;
    }

    template <typename T, std::uintptr_t Offset>
    typename pointer_offset<T, Offset>::address_type&
        get_value(pointer_offset<T, Offset>& v) noexcept
    {
        return v.get();
    }

    template <typename T, std::uintptr_t Offset>
    T& get_value(value_offset<T, Offset>& v) noexcept
    {
        return v.get();
    }
}