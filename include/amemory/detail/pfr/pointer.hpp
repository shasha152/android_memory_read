#pragma once

#include <type_traits>

#include "offset_proxy.hpp"

namespace amemory::detail
{
    template <typename T, std::uintptr_t Offset>
    class pointer_offset {
        T _pointer{};
        std::uintptr_t _remote_address{0};
        offset_proxy<Offset> _proxy;
    public:
        using value_type = T;
        using reference = T&;
        using const_reference = const T&;
        using pointer = T*;
        using const_pointer = const T*;
        using offset_type = offset_proxy<Offset>;
        using address_type = std::uintptr_t;
        using semantics = pointer_semantics;

        constexpr pointer_offset() noexcept = default;
        constexpr pointer_offset(std::uintptr_t remote_address) noexcept : _remote_address(remote_address) {}

        void set(value_type remote_address) noexcept { _remote_address = remote_address; }
        [[nodiscard]]
        address_type get() const noexcept { return _remote_address; }
        address_type& get() noexcept { return _remote_address; }

        reference operator*() noexcept { return _pointer; }
        const_reference operator*() const noexcept { return _pointer; }

        pointer operator->() noexcept
        {
            static_assert(std::is_class_v<T>, "value_type not is struct");
            return std::addressof(_pointer);
        }

        const_pointer operator->() const noexcept
        {
            static_assert(std::is_class_v<T>, "value_type not is struct");
            return std::addressof(_pointer);
        }

        void offset(std::uintptr_t off) noexcept
        {
            static_assert(!is_constexpr_offset<offset_type>::value, "offset is constexpr");
            _proxy.offset(off);
        }
        [[nodiscard]]
        constexpr std::uintptr_t offset() const noexcept { return _proxy.offset(); }

        [[nodiscard]]
        bool is_nullptr() const noexcept
        {
            return _remote_address != reinterpret_cast<address_type>(nullptr);
        }
        // operator std::uintptr_t&() noexcept { return _remote_address; }
        explicit operator bool() const noexcept
        {
            return _remote_address != reinterpret_cast<address_type>(nullptr);
        }
    };

    template <class T, std::uintptr_t Offset = static_cast<std::uintptr_t>(-1)>
    using pointer_offset_t = pointer_offset<T, Offset>;

    template <typename OffsetType, typename = void>
    inline constexpr bool is_offset_pointer_v = false;

    template <typename OffsetType>
    inline constexpr bool is_offset_pointer_v<
        OffsetType,
        std::enable_if_t<has_semantics_v<OffsetType>>
    > =
        is_pointer_semantics<
            typename std::remove_cvref_t<OffsetType>::semantics
        >::value;
}