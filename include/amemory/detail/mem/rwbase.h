#pragma once

#include "amemory/detail/base.h"

namespace amemory::detail
{

    // pread(int fd, uintptr_t address, void* data, std::size_t size)
    // syscall(long num, pid_t pid, iovce* r, std::size_t size, iovce* l, std::size_t size, long flag)
    template <typename Addr>
    struct vm_region
    {
        Addr address;
        std::size_t size;
        constexpr vm_region() noexcept = default;
        constexpr explicit vm_region(Addr addr, std::size_t size) : address(addr), size(size) {}
    };

    using local_address = void *;
    using remote_address = std::uintptr_t;

    using local_vm_region = vm_region<local_address>;
    using remote_vm_region = vm_region<remote_address>;

    struct rwdata
    {
        remote_address address;
        local_address data;
        std::size_t size;

        constexpr explicit operator local_vm_region() const noexcept
        {
            return local_vm_region{data, size};
        }

        constexpr explicit operator remote_vm_region() const noexcept
        {
            return remote_vm_region{address, size};
        }
    };

    template <typename T>
    struct redata
    {
        T value;
        remote_address address;
    };

    class rwbase : public base
    {
    public:
        using address_type = remote_address;
        rwbase() = delete;
        explicit rwbase(pid_t pid) noexcept : base(pid) {}
        ~rwbase() override = default;

        virtual void read(rwdata&) noexcept = 0;
        virtual void write(rwdata&) noexcept = 0;
    };
}
