#include "amemory/detail/parse/parse_maps.h"

#include <iostream>
#include <ostream>

#include "amemory/meta/algorithm_string.hpp"

namespace amemory::detail
{

    bool hex_to_int64(map_entry::address_type &address, char c) noexcept
    {
        bool result = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
        if (result)
            address = (address << 4) | (c >= 'a' ? (c - 'a' + 10) : (c - '0'));
        return result;
    }

    parse_maps::parse_maps(std::string_view maps_content)
        : parse_base(maps_content)
    {
    }

    parse_maps::value_type parse_maps::operator()()
    {
        return parse_line();
    }

    parse_maps::value_type parse_maps::parse_line()
    {
        value_type entry;
        next_token([&entry](char c){ return hex_to_int64(entry.start_address, c); });
        current_pos_++;
        next_token([&entry](char c){ return hex_to_int64(entry.end_address, c); });
        entry.permissions = get_token(4);
        for (int i = 0; i < 3; ++i)
            next_token();
        entry.pathname = next_token([](char c){ return c != '\n'; });
        return entry;
    }

    void parse_maps::skip_whitespace()
    {
        meta::skip_whitespace(content(), current_pos_);
    }

    meta::memory_t memory_type_parse::operator()(const map_entry& m) const
    {
        using memory_t = meta::memory_t;

        const auto path = m.path();

        if (m.per().empty())
            return memory_t::BAD;

        if (path.find("[stack") != std::string::npos)
            return memory_t::STACK;

        if (path.empty())
        {
            if (m.is_readable() && m.is_writable())
                return memory_t::ANONYMOUS;
        }

        if (path.find("/dev/ashmem") != std::string::npos)
            return memory_t::ASHMEM;

        if (path == "[heap]")
            return memory_t::C_HEAP;

        if (path.find("dalvik") != std::string::npos ||
            path.find("art") != std::string::npos)
        {
            if (m.is_writable())
                return memory_t::JAVA_HEAP;
            return memory_t::JAVA;
        }

        if (m.is_executable())
        {
            if (path.find("/system") != std::string::npos ||
                path.find("/apex")  != std::string::npos)
                return memory_t::CODE_SYS;

            if (path.find("/data") != std::string::npos)
                return memory_t::CODE_APP;
        }

        if (m.is_readable() && m.is_writable())
        {
            if (path.find(":.bss") != std::string::npos)
                return memory_t::C_BSS;

            if (path.find("/data") != std::string::npos)
                return memory_t::C_DATA;

            if (path.find("malloc") != std::string::npos)
                return memory_t::C_ALLOC;
        }

        if (path.find("kgsl") != std::string::npos ||
            path.find("mali") != std::string::npos ||
            path.find("gpu")  != std::string::npos)
            return memory_t::VIDEO;

        if (path.find("ppsspp") != std::string::npos)
            return memory_t::PPSSPP;

        return memory_t::OTHER;
    }
} // namespace amemory::detail
