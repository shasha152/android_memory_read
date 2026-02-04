#pragma once

#include <cassert>

#include "amemory/detail/mem/rwbase.h"

namespace amemory::detail
{
    class vm_readwrite;

    template <class Mem, typename Output>
    struct read_mem
    {
        Mem& mem;
        Output& out;

        explicit read_mem(Mem& mem, Output& cmp, std::size_t = 0) : mem(mem), out(cmp) {}

        template <typename Addr>
        void operator()(Addr start, void *buffer, std::size_t buffer_size)
        {
            rwdata data = {
                .address = start,
                .data = buffer,
                .size = buffer_size
            };
            mem.read(data);
            out(start, buffer, buffer_size);
        }

        void set_recycle()
        {

        }

        void flush() {}
    };

    template <typename Output>
    struct read_mem<vm_readwrite, Output>
    {
        std::vector<local_vm_region> locals;
        std::vector<remote_vm_region> remotes;
        std::size_t region_count = 0;
        int curr_count = 0;

        vm_readwrite& mem;
        Output& cmp;

        explicit read_mem(vm_readwrite& mem, Output& cmp, std::size_t count = 32) : mem(mem), cmp(cmp), region_count(
            std::min(count, static_cast<std::size_t>(1024)))
        {
            locals.reserve(region_count);
            remotes.reserve(region_count);
        }

        template <typename Addr>
        void operator()(Addr start, void *buffer, std::size_t buffer_size)
        {
            assert(curr_count < region_count);
            locals.emplace_back(buffer, buffer_size);
            remotes.emplace_back(start, buffer_size);

            curr_count++;
            if (curr_count == region_count)
                flush();
        }

        void flush()
        {
            if (curr_count == 0)
                return;
            mem.vm_read(locals.data(), remotes.data(), curr_count);
            for (int i = 0; i < curr_count; i++)
                cmp(remotes[i].address, locals[i].address, locals[i].size);
            curr_count = 0;
            locals.clear();
            remotes.clear();
        }

    };
}