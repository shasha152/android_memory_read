#pragma once

#include "rwbase.h"

namespace amemory::detail
{
    void process_vm_readv(pid_t pid, local_vm_region* remote_regions, remote_vm_region* local_regions, std::size_t count);
    void process_vm_writev(pid_t pid, local_vm_region* remote_regions, remote_vm_region* local_regions, std::size_t count);

    class vm_readwrite : public rwbase
    {
    public:
        explicit vm_readwrite(pid_t pid) noexcept;

        void read(rwdata &data) noexcept override;
        void write(rwdata &data) noexcept override;

        void vm_read(local_vm_region* remote_regions, remote_vm_region* local_regions, std::size_t count) noexcept;
        void vm_write(local_vm_region* remote_regions, remote_vm_region* local_regions, std::size_t count) noexcept;
    };
}