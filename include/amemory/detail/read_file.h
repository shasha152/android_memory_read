#pragma once

#include <cstddef>
#include <string_view>
#include <vector>

namespace amemory::detail
{
    ssize_t read_file(const std::string& path, void *buffer, std::size_t buffer_size);

    template <typename T>
    ssize_t read_file(const std::string& path, std::vector<T> &buffer, std::size_t)
    {
        return read_file(path, static_cast<void *>(buffer.data()), buffer.size() * sizeof(T));
    }

    template <typename T, std::size_t N>
    ssize_t read_file(const std::string& path, std::array<T, N> &buffer, std::size_t)
    {
        return read_file(path, static_cast<void *>(buffer.data()), buffer.size() * sizeof(T));
    }
}