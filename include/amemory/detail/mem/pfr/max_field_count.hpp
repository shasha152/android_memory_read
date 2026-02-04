#pragma once

#include <boost/pfr.hpp>

#include "amemory/detail/pfr/value.hpp"
#include "amemory/detail/pfr/pointer.hpp"

namespace amemory::detail
{
    constexpr std::size_t ct_max(std::size_t a) noexcept
    {
        return a;
    }

    template <typename... Ts>
    constexpr std::size_t ct_max(std::size_t a, Ts... rest) noexcept
    {
        constexpr auto b = ct_max(rest...);
        return a > b ? a : b;
    }

    template <typename Ty, std::size_t N, std::size_t ...I>
    struct pointer_count_ex2
    {
        static constexpr std::size_t value = pointer_count_ex2<Ty, I...>::value +
            static_cast<std::size_t>(is_offset_pointer_v<boost::pfr::tuple_element_t<N, Ty>>);
    };

    template <typename Ty, std::size_t N>
    struct pointer_count_ex2<Ty, N>
    {
        static constexpr std::size_t value = static_cast<std::size_t>(is_offset_pointer_v<boost::pfr::tuple_element_t<N, Ty>>);
    };

    template <typename Ty, typename >
    struct pointer_count_ex
    {
        static constexpr std::size_t value = 0;
    };

    template <typename Ty, std::size_t ...I>
    struct pointer_count_ex<Ty, std::index_sequence<I...>>
    {
        static constexpr std::size_t value = pointer_count_ex2<Ty, I...>::value;
    };


    template <typename Ty, typename = void>
    struct pointer_count
    {
        static constexpr std::size_t value = pointer_count_ex<Ty, std::make_index_sequence<
            boost::pfr::tuple_size_v<Ty>>>::value;
    };

    // template <typename Ty>
    // struct pointer_count<Ty, std::enable_if_t<!std::is_class_v<Ty>>>
    // {
    //     static constexpr std::size_t value = 1;
    // };

    template <typename Ty, std::size_t Field, std::size_t Out>
    constexpr void find_pointer_fill(
    std::array<std::size_t, pointer_count<Ty>::value>& arr) noexcept
    {
        if constexpr (Field < boost::pfr::tuple_size_v<Ty>)
        {
            if constexpr (is_offset_pointer_v<
                              boost::pfr::tuple_element_t<Field, Ty>>)
            {
                std::get<Out>(arr) = Field;
                find_pointer_fill<Ty, Field + 1, Out + 1>(arr);
            }
            else
            {
                find_pointer_fill<Ty, Field + 1, Out>(arr);
            }
        }
    }

    template <typename Ty>
    consteval std::array<std::size_t, pointer_count<Ty>::value> pointer_pos_array() noexcept
    {
        std::array<std::size_t, pointer_count<Ty>::value> arr = {};
        find_pointer_fill<Ty, 0, 0>(arr);
        return arr;
    }

    template <std::size_t... Is>
    struct index_list {};

    template <typename Ty, std::size_t I>
    struct pointer_list_proxy
    {
        using type = std::conditional_t<is_offset_pointer_v<
            boost::pfr::tuple_element_t<I, Ty>>,
            index_list<I>, index_list<>>;
    };

    template <typename, typename >
    struct index_concat;

    template <std::size_t... A, std::size_t... B>
    struct index_concat<index_list<A...>, index_list<B...>>
    {
        using type = index_list<A..., B...>;
    };

    template <typename...>
    struct index_concat_all;

    template <>
    struct index_concat_all<>
    {
        using type = index_list<>;
    };

    template <typename A>
    struct index_concat_all<A>
    {
        using type = A;
    };

    template <typename A, typename B>
    struct index_concat_all<A, B>
    {
        using type = typename index_concat<A, B>::type;
    };

    template <typename A, typename B, typename... Ts>
    struct index_concat_all<A, B, Ts...>
    {
        using type = typename index_concat_all<
            typename index_concat<A, B>::type, Ts...>::type;
    };

    template <typename, typename>
    struct pointer_list;

    template <typename Ty, std::size_t... I>
    struct pointer_list<Ty, std::index_sequence<I...>>
    {
        using type = typename index_concat_all<
            typename pointer_list_proxy<Ty, I>::type...>::type;
    };

    template <typename Ty>
    using pointer_list_t = typename pointer_list<Ty, std::make_index_sequence<
        boost::pfr::tuple_size_v<Ty>>>::type;

    template <typename T, typename = void>
    struct is_pfr_aggregate : std::false_type {};

    template <typename T>
    struct is_pfr_aggregate<
        T,
        std::void_t<
            decltype(boost::pfr::tuple_size_v<T>)
        >
    > : std::is_class<T> {};

    template <typename T>
    inline constexpr bool is_pfr_aggregate_v = is_pfr_aggregate<T>::value;

    template <typename Ty, typename = void>
    struct max_pointer_depth : std::integral_constant<std::size_t, 0> {};

    template <typename Ty>
    struct max_pointer_depth<
        Ty,
        std::enable_if_t<is_offset_pointer_v<Ty>>>
    : std::integral_constant<
          std::size_t,
          1 + max_pointer_depth<typename Ty::value_type>::value> {};

    template <typename Ty>
    struct max_pointer_depth<
        Ty,
        std::enable_if_t<
            !is_offset_pointer_v<Ty> &&
                !is_offset_value_v<Ty> &&
            is_pfr_aggregate<Ty>::value
        >>
    {
    private:
        template <std::size_t... I>
        static constexpr std::size_t calc(std::index_sequence<I...>) noexcept
        {
            return std::max({
                max_pointer_depth<
                    boost::pfr::tuple_element_t<I, Ty>>::value...
            });
        }

    public:
        static constexpr std::size_t value =
            calc(std::make_index_sequence<boost::pfr::tuple_size_v<Ty>>{});
    };

    template <typename T, std::size_t D, std::size_t N>
    constexpr void max_field_pos_array(std::array<std::size_t, D>& depth_);

    template <typename Ty, std::size_t D, std::size_t N>
    constexpr void max_field_count_proxy(std::array<std::size_t, D>& depth_) noexcept
    {
        if constexpr (D != N)
        {
            if constexpr (is_pfr_aggregate<Ty>::value)
            {
                std::size_t size = boost::pfr::tuple_size_v<Ty>;
                depth_[N] += size;
            }
            max_field_pos_array<Ty, D, N + 1>(depth_);
        }
    }

    template <typename Ty>
    consteval std::size_t max_field_count() noexcept
    {
        std::array<std::size_t, max_pointer_depth<Ty>::value + 1> depth = { 0 };
        max_field_count_proxy<Ty, max_pointer_depth<Ty>::value + 1, 0>(depth);
        return *std::max_element(depth.begin(), depth.end());
    }

    template <typename T, std::size_t D, std::size_t N>
    constexpr void max_field_pos_array(std::array<std::size_t, D>& depth_)
    {
        [&]<std::size_t... I>(index_list<I...>) {
            (max_field_count_proxy<
                typename boost::pfr::tuple_element_t<I, T>::value_type, D, N>(depth_), ...);
        }(pointer_list_t<T>{});
    }

    template <typename Ty>
    inline constexpr auto pointer_count_v = pointer_count<Ty>::value;

    template <typename Ty>
    inline constexpr auto pointer_pos_array_v = pointer_pos_array<Ty>();

    template <typename Ty>
    inline constexpr auto max_pointer_depth_v = max_pointer_depth<Ty>::value;

    template <typename Ty>
    inline constexpr auto max_field_count_v = max_field_count<Ty>();
}