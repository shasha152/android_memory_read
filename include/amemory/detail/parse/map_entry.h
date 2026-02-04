#pragma once

#include <string_view>

#include "amemory/meta/memory_t.h"

#ifndef MEMORY_NODISCARD
#define MEMORY_NODISCARD [[nodiscard]]
#endif

namespace amemory::detail
{
    struct map_entry
    {
        using address_type = std::uintptr_t;

        address_type start_address = 0;
        address_type end_address = 0;
        std::string_view permissions; // "rwxp"
        std::string_view pathname;

        MEMORY_NODISCARD std::string_view per() const noexcept
        {
            return permissions;
        }

        MEMORY_NODISCARD address_type start() const noexcept
        {
            return start_address;
        }

        MEMORY_NODISCARD address_type end() const noexcept
        {
            return end_address;
        }

        MEMORY_NODISCARD bool is_readable() const noexcept
        {
            return permissions[0] == 'r';
        }

        MEMORY_NODISCARD bool is_writable() const noexcept
        {
            return permissions[1] == 'w';
        }

        MEMORY_NODISCARD bool is_executable() const noexcept
        {
            return permissions[2] == 'x';
        }

        MEMORY_NODISCARD bool is_private() const noexcept
        {
            return permissions[3] == 'p';
        }

        MEMORY_NODISCARD bool is_shared() const noexcept
        {
            return permissions[3] == 's';
        }

        MEMORY_NODISCARD std::string_view path() const noexcept
        {
            return pathname;
        }
    };

    struct memory_type_parse {
        meta::memory_t operator()(const map_entry& m) const; };
}