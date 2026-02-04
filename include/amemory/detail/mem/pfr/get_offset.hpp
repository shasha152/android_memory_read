#pragma once

#include <boost/pfr.hpp>
#include "max_field_count.hpp"


namespace amemory::detail
{
    template <typename T, typename = void>
    inline constexpr bool has_declared_offset_v =
        (is_offset_pointer_v<T> || is_offset_value_v<T>) &&
        is_constexpr_offset_v<T>;

    template <typename T, std::size_t N>
    inline constexpr bool has_declared_offset_v_n = (is_offset_pointer_v<boost::pfr::tuple_element_t<N, T>> || is_offset_value_v<boost::pfr::tuple_element_t<N, T>>)
        && is_constexpr_offset_v<boost::pfr::tuple_element_t<N, T>>;

    template <typename T, typename = void>
    struct field_size_proxy : std::integral_constant<std::size_t, sizeof(T)> {};

    template <typename T>
    struct field_size_proxy<T, std::enable_if_t<
        is_offset_pointer_v<T>>> : std::integral_constant<std::size_t, sizeof(typename T::address_type)> {};

    template <typename T>
    struct field_size_proxy<T, std::enable_if_t<
        is_offset_value_v<T>>> : std::integral_constant<std::size_t, sizeof(typename T::value_type)> {};

    template <typename Ty, std::size_t N>
    struct field_size
    {
        using value_type = boost::pfr::tuple_element_t<N, Ty>;
        static constexpr std::size_t size = field_size_proxy<value_type>::value;
    };

    template <std::size_t N>
    struct field_size<void, N>
    {
        using value_type = void;
        static constexpr std::size_t size = 0;
    };

    template <typename T, std::size_t N, typename = void>
    struct get_offset : field_size<T, N>
    {
        static constexpr std::uintptr_t value = get_offset<T, N - 1>::value + get_offset<T, N - 1>::size;
    };

    template <typename T, std::size_t N>
    struct get_offset<T, N, std::enable_if_t<
        has_declared_offset_v_n<T, N>>>
    : field_size<T, N> {
        static constexpr std::uintptr_t value = field_size<T, N>::value_type::offset_type::offset();
    };

    template <typename T>
    struct get_offset<T, 0, std::enable_if_t<
        !has_declared_offset_v_n<T, 0>>>
    : field_size<void, 0> {
        static constexpr std::uintptr_t value = 0;
    };

    template <typename T, std::size_t N>
    inline constexpr auto get_offset_v = get_offset<T, N>::value;
}