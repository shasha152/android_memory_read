#pragma once

#include <cstdint>

namespace amemory::detail
{
    template <std::uintptr_t Offset>
    struct offset_proxy
    {
        static constexpr auto _offset = Offset;
        [[nodiscard]]
        static constexpr std::uintptr_t offset() noexcept { return _offset; }
    };

    template <>
    struct offset_proxy<static_cast<std::uintptr_t>(-1)>
    {
        void offset(std::uintptr_t off) noexcept { _offset = off; }
        [[nodiscard]]
        std::uintptr_t offset() const noexcept { return _offset; }

        std::uintptr_t _offset = 0;
    };

    template <class OffsetType, typename = void>
    struct is_constexpr_offset : std::false_type {};

    template <class OffsetType>
    struct is_constexpr_offset<OffsetType,
        std::void_t<decltype(OffsetType::offset_type::_offset)>> : std::true_type {};

    template <class OffsetType>
    inline constexpr bool is_constexpr_offset_v = is_constexpr_offset<OffsetType>::value;

    struct value_semantics
    {
        static constexpr bool is_value = true;
        static constexpr bool is_pointer = false;
    };

    struct pointer_semantics
    {
        static constexpr bool is_value = false;
        static constexpr bool is_pointer = true;
    };

    template <typename , typename = void>
    struct has_semantics : std::false_type {};
    template <typename Ty>
    struct has_semantics<Ty, std::void_t<typename Ty::semantics>> : std::true_type {};

    template <typename Ty>
    inline constexpr bool has_semantics_v = has_semantics<std::remove_cvref_t<Ty>>::value;

    template <typename, typename = void>
    struct has_value_semantics : std::false_type {};
    template <typename Semantics>
    struct has_value_semantics<Semantics, std::void_t<decltype(Semantics::is_value)>> : std::true_type {};

    template <typename, typename = void>
    struct has_pointer_semantics : std::false_type {};
    template <typename Semantics>
    struct has_pointer_semantics<Semantics, std::void_t<decltype(Semantics::is_pointer)>> : std::true_type {};

    template <typename Semantics>
    struct is_pointer_semantics : std::bool_constant<Semantics::is_pointer && has_pointer_semantics<Semantics>::value> {};
    template <typename Semantics>
    struct is_value_semantics : std::bool_constant<Semantics::is_value && has_value_semantics<Semantics>::value> {};
}