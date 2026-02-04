#pragma once

#include "get_offset.hpp"


namespace amemory::detail
{
    template <typename T, std::size_t N>
    struct check_offset_field
    {
        static constexpr bool ok = get_offset_v<T, N> >
                                    get_offset_v<T, static_cast<std::size_t>(N - 1)>;

        static_assert(ok, "offset is out of range");
    };

    template <typename T>
    struct check_offset_field<T, 0>
    {
        static constexpr bool ok = true;
    };

    template <typename T>
    struct layout_validator
    {
    private:

        template <std::size_t...I>
        consteval static void calc(std::index_sequence<I...>) noexcept
        {
            (static_cast<void>(check_offset_field<T, I>::ok), ...);
        }
    public:
        consteval static void check() noexcept
        {
            calc(std::make_index_sequence<
                boost::pfr::tuple_size_v<T>>{});
        }
    };
}