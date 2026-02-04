#pragma once

#include <cctype>
#include <string_view>

namespace amemory::meta
{
    inline bool is_number(std::string_view str) noexcept
    {
        return std::all_of(str.begin(), str.end(), isdigit);;
    }

    inline std::int64_t to_int64(std::string_view str) noexcept
    {
        std::int64_t result = 0;
        for (const char c : str)
        {
            if (c < '0' || c > '9') return -1;
            result = result * 10 + (c - '0');
        }

        return result;
    }

    inline void skip_whitespace(std::string_view str, std::size_t& pos) noexcept
    {
        while (pos < str.size() &&
                       (str[pos] == ' ' || str[pos] == '\t'))
        {
            ++pos;
        }
    }
}