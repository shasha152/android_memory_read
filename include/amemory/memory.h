#pragma once

#include "amemory/detail/pfr/for_each.hpp"
#include "amemory/detail/mem/vm_readwrite.h"
#include "detail/mem/pfr/manager.hpp"


namespace amemory
{

#if defined(ANDROID) || defined(LINUX)
    using default_access = detail::vm_readwrite;
#elif defined(WIN32)
#error "WINDOWS"
#endif

    template <typename M>
    using manager = detail::manager<M>;
    // template <typename Access, typename T>
    // void read(Access& m, std::uintptr_t address, T& value) noexcept
    // {
    //     handle_read<Access, T> _handle_read(m, address);
    //     detail::for_each_offset(value, detail::offset_for_each_visitor<T,
    //         handle_read<Access, T>&&>(std::move(_handle_read)));
    // }
    namespace detail { inline constexpr auto default_offset = static_cast<std::uintptr_t>(-1); }

    template <typename Type, std::uintptr_t offset = detail::default_offset>
    using offset_t = detail::value_offset<Type, offset>;

    template <std::uintptr_t offset = detail::default_offset>
    using offset_int8_t = offset_t<std::int8_t, offset>;

    template <std::uintptr_t offset = detail::default_offset>
    using offset_int16_t = offset_t<std::int16_t, offset>;

    template <std::uintptr_t offset = detail::default_offset>
    using offset_int32_t = offset_t<std::int32_t, offset>;

    template <std::uintptr_t offset = detail::default_offset>
    using offset_float_t = offset_t<float, offset>;

    template <std::uintptr_t offset = detail::default_offset>
    using offset_double_t = offset_t<double, offset>;

    template <typename Type, std::uintptr_t offset = detail::default_offset>
    using pointer_t = detail::pointer_offset_t<Type, offset>;
}
