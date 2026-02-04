#pragma once

#include <array>

namespace amemory::detail
{
    template <typename Ty>
    struct two_value
    {
        using value_type = Ty;

        std::array<Ty, 2> value;

        two_value() = default;

        explicit constexpr operator std::pair<Ty, Ty>() const noexcept
        { return std::pair<Ty, Ty>(value[0], value[1]); }

        template <std::size_t Index>
        constexpr Ty& get() const noexcept { return value[Index]; }

        Ty* data() noexcept { return &value[0]; }

    };
}