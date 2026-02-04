//
// Created by 30980 on 2025/12/16.
//

#include <fcntl.h>
#include <unistd.h>

#include "amemory/detail/mem/vm_readwrite.h"
#include "amemory/detail/mem/p_readwrite.h"

#include <sys/uio.h>
#include <sys/syscall.h>


static_assert(sizeof(amemory::detail::vm_region<void *>) == sizeof(iovec), "vm_region格式错误");

namespace amemory::detail
{
    static void _process_vm(long number, pid_t pid, local_vm_region* remote_regions, remote_vm_region* local_regions, size_t count)
    {
        constexpr std::size_t max_processes = UIO_MAXIOV;

        std::size_t processes = count / max_processes;
        std::size_t remaining = count % max_processes;
        std::size_t process = 0;

        while (processes)
        {
            syscall(number, pid, reinterpret_cast<iovec *>(remote_regions + process), max_processes, reinterpret_cast<iovec *>(local_regions + process), max_processes, 0);
            process += max_processes;
            processes--;
        }
        if (remaining)
            syscall(number, pid, reinterpret_cast<iovec *>(remote_regions + process), remaining, reinterpret_cast<iovec *>(local_regions + process), remaining, 0);
    }

    void process_vm_readv(pid_t pid, local_vm_region* remote_regions, remote_vm_region* local_regions, std::size_t count)
    {
        _process_vm(SYS_process_vm_readv, pid, remote_regions, local_regions, count);
    }

    void process_vm_writev(pid_t pid, local_vm_region* remote_regions, remote_vm_region* local_regions, std::size_t count)
    {
        _process_vm(SYS_process_vm_writev, pid, remote_regions, local_regions, count);
    }

    vm_readwrite::vm_readwrite(pid_t pid) noexcept
        : rwbase(pid)
    {
    }


    void vm_readwrite::read(rwdata& data) noexcept
    {
        auto l = static_cast<local_vm_region>(data);
        auto r = static_cast<remote_vm_region>(data);
        process_vm_readv(pid(), &l, &r, 1);
    }

    void vm_readwrite::write(rwdata& data) noexcept
    {
        auto l = static_cast<local_vm_region>(data);
        auto r = static_cast<remote_vm_region>(data);
        process_vm_writev(pid(), &l, &r, 1);
    }

    void vm_readwrite::vm_read(local_vm_region* remote_regions, remote_vm_region* local_regions, std::size_t count) noexcept
    {
        process_vm_readv(pid(), remote_regions, local_regions, count);
    }

    void vm_readwrite::vm_write(local_vm_region* remote_regions, remote_vm_region* local_regions, std::size_t count) noexcept
    {
        process_vm_writev(pid(), remote_regions, local_regions, count);
    }

    p_readwrite& p_readwrite::operator=(const p_readwrite& other)
    {
        destroy();
        auto mem_path = std::string(path()) + "/mem";
        fd_ = open(mem_path.c_str(), O_RDWR);
        return *this;
    }

    p_readwrite& p_readwrite::operator=(p_readwrite&& other) noexcept
    {
        destroy();
        fd_ = other.fd_;
        other.fd_ = -1;
        return *this;
    }

    p_readwrite::p_readwrite(pid_t pid) : rwbase(pid)
    {
        auto mem_path = std::string(path()) + "/mem";
        fd_ = open(mem_path.c_str(), O_RDWR);
    }

    p_readwrite::p_readwrite(const p_readwrite& other) : rwbase(other.pid())
    {
        *this = other;
    }

    p_readwrite::p_readwrite(p_readwrite&& other) noexcept : rwbase(other.pid())
    {
        *this = std::move(other);
    }

    void p_readwrite::read(rwdata& datas) noexcept
    {
        syscall(SYS_pread64, fd_, datas.data, datas.size, datas.address);
    }

    void p_readwrite::write(rwdata& datas) noexcept
    {
        syscall(SYS_pwrite64, fd_, datas.data, datas.size, datas.address);
    }

    void p_readwrite::destroy() noexcept
    {
        if (fd_ != -1)
        {
            close(fd_);
            fd_ = -1;
        }
    }

    p_readwrite::~p_readwrite()
    {
        destroy();
    }
}
