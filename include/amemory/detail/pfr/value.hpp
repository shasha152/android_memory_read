#pragma once

#include <utility>
#include <type_traits>

#include "offset_proxy.hpp"

namespace amemory::detail
{

    template <class T, std::uintptr_t Offset>
    class value_offset
    {
        T _value{};
        offset_proxy<Offset> _proxy;
    public:
        using value_type = T;
        using reference = T&;
        using const_reference = const T&;
        using offset_type = offset_proxy<Offset>;

        using semantics = value_semantics;

        constexpr value_offset() noexcept = default;
        constexpr value_offset(const T& v) noexcept : _value(v) {}
        constexpr value_offset(T&& v) noexcept : _value(std::move(v)) {}

        const_reference get() const noexcept { return _value; }
        reference get() noexcept { return _value; }

        template <class V>
        void set(V&& v) noexcept
        {
            _value = std::forward<V>(v);
        }

        void offset(std::uintptr_t off) noexcept
        {
            static_assert(!is_constexpr_offset<offset_type, void>::value, "offset is constexpr");
            _proxy.offset(off);
        }
        [[nodiscard]]
        constexpr std::uintptr_t offset() const noexcept { return _proxy.offset(); }

        constexpr operator reference() { return _value; }
        constexpr operator const_reference() const { return _value; }
    };

    template <class T, std::uintptr_t Offset = static_cast<std::uintptr_t>(-1)>
    using value_offset_t = value_offset<T, Offset>;

    template <typename OffsetType, typename = void>
    inline constexpr bool is_offset_value_v = false;

    template <typename OffsetType>
    inline constexpr bool is_offset_value_v<
        OffsetType,
        std::enable_if_t<has_semantics_v<OffsetType>>
    > =
        is_value_semantics<
            typename std::remove_cvref_t<OffsetType>::semantics
        >::value;
}


#undef VALUE_STRUCT