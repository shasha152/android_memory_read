#pragma once

#include <bits/signal_types.h>
#include <boost/pfr.hpp>
#include "value.hpp"
#include "pointer.hpp"

namespace amemory::detail
{

    template <typename T, typename = void>
    struct value_type_size
    {
        constexpr static std::size_t value = sizeof(T);
    };

    template <typename T>
    struct value_type_size<T, std::void_t<typename T::value_type>>
    {
        constexpr static std::size_t value = sizeof(typename T::value_type);
    };

    enum class field_type
    {
        normal,
        value,
        pointer
    };

    template <typename Field, typename = void>
    inline constexpr auto field_flag_v = field_type::normal;

    template <typename Field>
    inline constexpr auto field_flag_v<Field, std::enable_if_t<
        is_offset_value_v<Field>
    >> = field_type::value;

    template <typename Field>
    inline constexpr auto field_flag_v<Field, std::enable_if_t<
        is_offset_pointer_v<Field>
    >> = field_type::pointer;

    template <typename Struct, std::size_t Index>
    struct value_group
    {
        using struct_type = Struct;
        using value_type = boost::pfr::tuple_element_t<Index, Struct>;
        struct_type& value;

        constexpr explicit value_group(struct_type& value) : value(value) {}

        [[nodiscard]]
        static constexpr std::size_t index() noexcept { return Index; }

        [[nodiscard]]
        value_type& get() noexcept
        {
            return boost::pfr::get<Index>(value);
        }

        [[nodiscard]]
        static constexpr std::size_t value_size() noexcept
        {
            return value_type_size<value_type>::value;
        }

        [[nodiscard]]
        static constexpr field_type flag() noexcept
        {
            return field_flag_v<value_type>;
        }
    };

    template <class T, class F>
    void for_each_offset(T& value, F&& func)
    {
        [&]<std::size_t... I>(std::index_sequence<I...>)
        {
            (std::forward<F>(func)
                .template next<T, I>(value_group<T, I>(value)), ...);
        }(std::make_index_sequence<boost::pfr::tuple_size_v<T>>{});
    }
}
