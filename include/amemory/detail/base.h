#pragma once

#include <cstdint>
#include <string>
#include <filesystem>

namespace amemory
{
    struct self_t
    {
        constexpr self_t() noexcept = default;
        constexpr operator pid_t() const noexcept { return -1; }
    };

    inline constexpr self_t self{};
namespace detail
{
    template <std::size_t Size, typename Allocator = std::allocator<char>>
    struct data {
        using alloc_traits = std::allocator_traits<Allocator>;

        struct deleter {
            Allocator* alloc;
            void operator()(char* p) const noexcept {
                if (p) alloc_traits::deallocate(*alloc, p, Size);
            }
        };

        Allocator _allocator;
        std::unique_ptr<char, deleter> _data;

        data()
            : _data(alloc_traits::allocate(_allocator, Size), deleter{&_allocator})
        {}

        [[nodiscard]]
        char* get_data() const noexcept { return _data.get(); }

        [[nodiscard]]
        constexpr std::size_t max_size() const noexcept { return Size; }
    };

    class base
    {
        pid_t pid_{};
        std::string path_;
    public:
        explicit base(pid_t pid) : pid_(pid)
        {
            if (pid_ == -1) path_ = "/proc/self";
            else
                path_ = "/proc/" + std::to_string(pid_);
        }

        explicit base(std::string_view path) : path_(path)
        {
        }

        virtual ~base() = default;

        [[nodiscard]]
        pid_t pid() const noexcept
        {
            return pid_;
        }

        [[nodiscard]]
        std::string_view path() const
        {
            return path_;
        }

        [[nodiscard]]
        bool has_pid() const noexcept
        {
            return std::filesystem::is_directory(path_);
        }
    };
}
} // namespace
