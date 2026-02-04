#pragma once

#include <string_view>
#include <string>
#include <vector>
#include <array>

namespace amemory::meta
{
    inline std::string_view to_string_view(const std::string &str, std::size_t size)
    {
        return std::string_view(str);
    }

    inline std::string_view to_string_view(const char *data, std::size_t size)
    {
        return std::string_view{data, size};
    }

    template <std::size_t Size>
    inline std::string_view to_string_view(const std::array<char, Size> &data, std::size_t size)
    {
        return std::string_view{data.data(), size};
    }

    template <typename T>
    inline std::string_view to_string_view(const T *data, std::size_t size)
    {
        return std::string_view{reinterpret_cast<const char *>(data), size * sizeof(T)};
    }
} // namespace amemory::meta
