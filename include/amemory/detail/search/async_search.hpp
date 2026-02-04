#pragma once

#include "amemory/detail/mem/rwbase.h"
#include "amemory/detail/parse/map_entry.h"
#include "amemory/meta/thread_pool.hpp"
#include "memory_splitter.hpp"
#include "read_mem.hpp"
#include "compare.hpp"

namespace amemory::detail
{
    template <typename Pool>
    struct async_pool
    {
        inline static Pool pool;

        template <typename Fun, typename... Args>
        auto submit(Fun call, Args&&... args)
        {
            return pool.submit(call, std::forward<Args>(args)...);
        }
    };

    struct wait_condition
    {
        std::condition_variable cv;
        std::mutex mutex;
        std::atomic_int64_t counter = 0;
    };

    template <typename Pid, typename Buffer, typename Thread, typename Mem>
    struct tool_set
    {
        Pid pid;
        Buffer& buffer;
        Thread& thread;
        Mem& mem;
        wait_condition& wait_condition;

        using mem_type = Mem;
        using buffer_type = Buffer;
    };

    template <typename Mem>
    using mem_address_type = typename Mem::address_type;

    template <typename Tp>
    struct is_float : std::false_type {};

    template <> struct is_float<float> : std::true_type {};
    template <> struct is_float<double> : std::true_type {};

    template <typename Tp>
    inline constexpr bool is_float_v = is_float<std::remove_cvref_t<Tp>>::value;

    template <typename ReadMem, typename = void>
    struct notice
    {
        void operator()(ReadMem&) const noexcept {}
    };

    template <typename ReadMem>
    struct notice<ReadMem, std::void_t<decltype(std::declval<ReadMem>().flush())>>
    {
        void operator()(ReadMem& read_mem) const noexcept
        {
            read_mem.flush();
        }
    };

    template <typename Tp, typename Compare, typename OnMach, typename Buffer>
    struct fwd_compare
    {
        Compare comp;
        Tp value;
        OnMach mach;
        Buffer& buffer;

        template <typename _OnMach>
        explicit fwd_compare(const Tp& value, _OnMach&& mach, Buffer& buffer) noexcept :
            value(value), mach(std::forward<_OnMach>(mach)), buffer(buffer)
        {
        }

        void operator()(remote_address raddress, local_address laddress, std::size_t size) noexcept
        {
            out_find_address out_find_address(*this, laddress, raddress);
            comp(value, laddress, size, out_find_address);
            buffer.release(typename Buffer::buffer_type(static_cast<std::uint8_t* >(laddress)));
        }

    private:

        struct out_find_address
        {
            local_address address1;
            remote_address address2;
            fwd_compare& self;

            explicit out_find_address(fwd_compare& _this, local_address address, remote_address _address) noexcept :
                address1(address), address2(_address), self(_this) {}

            void operator()(local_address address) const noexcept
            {
                auto count = static_cast<std::size_t>(static_cast<char*>(address) - static_cast<char*>(address1));
                self.mach(redata<Tp>{self.value, static_cast<remote_address>(static_cast<std::uintptr_t>(address2) + count)});
            }
        };
    };

    template <typename Tp, typename Values, typename OnMachFun, typename Tools>
    void async_search(Tp val, Values&& be, OnMachFun&& on_mach, Tools tools)
    {
        // using mem_type = typename Tools::mem_type;
        // using compare_type = fwd_compare<Tp, compare<Tp>, OnMachFun, typename Tools::buffer_type>;
        // using notice_type = notice<read_mem<mem_type, compare_type>>;
        // auto submit = [=, cmp = compare_type(val, std::forward<OnMachFun>(on_mach), tools.buffer)]()
        // mutable {
        //     read_mem<mem_type, compare_type> mem(tools.mem, cmp, tools.buffer.buffer_count() / 2);
        //     memory_splitter splitter;
        //     splitter(std::forward<Values>(be), tools.buffer,
        //         [&mem, &tools](auto addr, auto buffer, std::size_t size) { mem(addr, buffer.get(), size); },
        //         [&mem, &tools](){ notice_type()(mem); });
        //     notice_type()(mem);
        // };
        // tools.thread.submit(submit);

        struct comp
        {
            void operator()(remote_address raddress, local_address laddress, std::size_t size) const noexcept
            {
#pragma clang loop vectorize(enable)
#pragma GCC ivdep
                for (std::size_t i = 0; i < size; i += 4)
                {
                    int value = *reinterpret_cast<std::int32_t*>(static_cast<char *>(laddress) + i);
                    if (value == 10000)
                        std::cout << "RemoteAddr: " << std::hex << raddress + i << std::endl;
                }
            }
        };
        // memory_splitter splitter;
        // auto submit = [&be, &tools, &splitter]()
        // {
        //     splitter(std::forward<Values>(be), tools.buffer,
        //         [&](auto addr, auto buffer, std::size_t size)
        //         {
        //             // std::cout << std::hex << "Remote: " << addr << " Buffer: " << reinterpret_cast<void *>(buffer.get()) << " Size: " << std::dec << size << std::endl;
        //             comp comp_;
        //             read_mem<typename Tools::mem_type, comp> read_mem(tools.mem, comp_);
        //             read_mem(addr, buffer.get(), size);
        //             tools.buffer.release(std::move(buffer));
        //         },[&]()
        //         {
        //             std::cout << "Not Buffer" << std::endl;
        //         }
        //     );
        //
        //     std::cout << "END" << std::endl;
        // };

        memory_splitter splitter;
        comp comp_;
        auto rm =  std::make_shared<read_mem<typename Tools::mem_type, comp>>(tools.mem, comp_);

        splitter(std::forward<Values>(be), tools.buffer.buffer_size(),
            [&](auto addr, std::size_t size)
            {
                tools.wait_condition.counter.fetch_add(1, std::memory_order_relaxed);
                tools.thread.submit([addr, size, tools, rm]()
                {
                    auto buf = tools.buffer.acquire([&rm]
                    {
                        rm->flush();
                    });

                    *rm(addr, buf.get(), size);
                    tools.buffer.release(std::move(buf));
                    tools.wait_condition.counter.fetch_sub(1, std::memory_order_release);
                    tools.wait_condition.cv.notify_one();
                });
            }
        );
    }
}