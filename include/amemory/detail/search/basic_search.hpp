#pragma once

#include <unordered_map>
#include <thread>

#include "amemory/detail/mem/rwbase.h"
#include "amemory/detail/parse/map_entry.h"
#include "amemory/meta/buffer_pool.hpp"
#include "amemory/meta/memory_t.h"

#include "async_search.hpp"
#include "two_value.hpp"

namespace amemory::detail
{
    std::size_t page_size();

    template <typename Mem, typename = void>
    struct is_mem : std::false_type {};

    template <typename Mem>
    struct is_mem<Mem, std::enable_if_t<std::is_base_of_v<rwbase, Mem>>
    > : std::true_type {};

    template <typename Mem>
    inline constexpr bool is_mem_v = is_mem<std::remove_cvref_t<Mem>>::value;

    template <typename BufferPool, typename = void>
    struct is_buffer_pool : std::false_type {};

    template <typename BufferPool>
    struct is_buffer_pool<BufferPool, std::void_t<
        decltype(std::declval<BufferPool>().acquire()),
        decltype(std::declval<BufferPool>().release(
            std::declval<BufferPool>().acquire()
        ))
    >> : std::true_type {};

    template <typename BufferPool>
    inline constexpr bool is_buffer_pool_v = is_buffer_pool<std::remove_cvref_t<BufferPool>>::value;

    template <typename Bench, typename Addr>
        struct forward_bench : two_value<Addr>
    {

        template <typename Bench_Type,
            typename = std::enable_if_t<std::is_same_v<std::remove_cv_t<Bench_Type>, Bench>>>
        explicit forward_bench(Bench_Type&& b, Bench_Type&&) noexcept
        {
            auto b1 = std::forward<Bench_Type>(b);
            two_value<Addr>::value[0] = b1.start();
            two_value<Addr>::value[1] = b1.end();
        }
    };

    template <typename Addr>
    struct forward_bench<map_entry::address_type, Addr> : two_value<Addr>
    {
        explicit forward_bench(map_entry::address_type b, map_entry::address_type e) noexcept
        {
            two_value<Addr>::value[0] = b;
            two_value<Addr>::value[1] = e;
        }
    };

    template <class Mem,
        class BufferPool = meta::buffer_pool, class ThreadPool = meta::thread_pool, class MapToTypeParse = memory_type_parse>
    class basic_search : public base
    {
        static_assert(is_mem_v<Mem>, "memory type is not a memory type");
        static_assert(is_buffer_pool_v<BufferPool>, "buffer pool is not a memory type");

        Mem mem_;
        BufferPool buffer_pool_;
        int memory_type_;
        int curr_id_;
        async_pool<ThreadPool> thread_pool_;
        std::unordered_map<int, wait_condition> wait_conditions_;

        bool is_continue_ = false;

        using tools_type = tool_set<pid_t, BufferPool, async_pool<ThreadPool>, Mem>;
    public:
        using address_type = typename Mem::address_type;
        using wait_id_type = typename std::unordered_map<int, wait_condition>::key_type;

        explicit basic_search(pid_t pid, std::size_t buffer_size = 1024 * 32, std::size_t buffer_count = 32) :
            base(pid), mem_(pid), buffer_pool_(buffer_size, buffer_count), memory_type_(static_cast<int>(meta::memory_t::UNKNOWN)), curr_id_(0)
        {
            assert(buffer_size % page_size() == 0);
        }

        explicit basic_search(Mem mem, std::size_t buffer_size = 1024 * 32, std::size_t buffer_count = 32) noexcept :
            base(mem.pid()), mem_(std::move(mem)), buffer_pool_(buffer_size, buffer_count), memory_type_(static_cast<int>(meta::memory_t::UNKNOWN)), curr_id_(0)
        {
            assert(buffer_size % page_size() == 0);
        }

        explicit basic_search(const Mem& mem, std::size_t buffer_size = 1024 * 32, std::size_t buffer_count = 32) :
            base(mem.pid()), mem_(mem), buffer_pool_(buffer_size, buffer_count), memory_type_(static_cast<int>(meta::memory_t::UNKNOWN)), curr_id_(0)
        {
            assert(buffer_size % page_size() == 0);
        }

        template <typename Tp, typename OnMachFun, typename Bench>
        wait_id_type search(Tp val, Bench&& start, Bench&& stop, OnMachFun&& fun);

        void set_ranges(int memory_type) noexcept
        {
            memory_type_ = memory_type;
        }

        Mem memory() const noexcept
        {
            return mem_;
        }

        BufferPool buffer_pool() const noexcept
        {
            return buffer_pool_;
        }

        void wait(wait_id_type id) noexcept
        {
            auto& waiter = wait_conditions_[id];
            std::unique_lock lock(waiter.mutex);
            waiter.cv.wait(lock, [&waiter]() { return waiter.counter == 0; });
            lock.unlock();
            wait_conditions_.erase(id);
        }

        void is_continue_page(bool is) noexcept
        {
            is_continue_ = is;
        }
    private:

        wait_id_type create_waiter() noexcept
        {
            curr_id_++;
            wait_conditions_.try_emplace(curr_id_);

            return curr_id_;
        }

        template <class Parse_>
        struct _curr_maps
        {
            basic_search& self;
            Parse_ map_to_type_parse_;

            explicit _curr_maps(basic_search& self) : self(self) {}

            template <typename MapIter, typename Foreach>
            void for_each_maps(MapIter b, MapIter e, Foreach foreach,
                std::enable_if_t<!std::is_same_v<
                    map_entry::address_type, std::remove_cvref_t<MapIter>>, int> enableIf = 0)
            {
                const int memory_type = self.memory_type_;
                if (memory_type == static_cast<int>(meta::memory_t::EMPTY))
                    return;
                if (memory_type == static_cast<int>(meta::memory_t::UNKNOWN))
                {
                    for (; b != e; ++b)
                        foreach(std::move(*b));
                }
                else
                {
                    for (; b != e; ++b)
                    {
                        auto _map = *b;
                        if (memory_type & static_cast<int>(map_to_type_parse_(_map)))
                            foreach(std::move(_map));
                    }
                }
            }

            template <typename MapIter, typename Foreach>
            void for_each_maps(MapIter b, MapIter e, Foreach foreach,
                std::enable_if_t<std::is_same_v<
                    map_entry::address_type, std::remove_cvref_t<MapIter>>, int> enableIf = 0)
            {
                foreach(b, e);
            }
        };
    };


    template <class Mem, class BufferPool, class ThreadPool, class MapToTypeParse>
    template <typename Tp, typename OnMachFun, typename Bench>
        typename basic_search<Mem, BufferPool, ThreadPool, MapToTypeParse>::wait_id_type
    basic_search<Mem, BufferPool, ThreadPool, MapToTypeParse>::search(Tp val, Bench&& start, Bench&& stop, OnMachFun&& fun)
    {
        auto id = create_waiter();
        auto maps_for = _curr_maps<MapToTypeParse>(*this);
        maps_for.for_each_maps(start, stop, [this, &val, &fun, id]<typename T0>(T0&& _start, T0&& _stop = T0{})
        {
            forward_bench<T0, address_type> forward_bench(std::forward<T0>(_start), std::forward<T0>(_stop));
            async_search(std::move(val), static_cast<two_value<address_type>&&>(forward_bench), std::forward<OnMachFun>(fun),
                tools_type{pid(), buffer_pool_, thread_pool_, mem_, wait_conditions_[id]});
        });
        return id;
    }


    template <class Tp, class Allocator = std::allocator<Tp>, class Vector_ = std::vector<Tp, Allocator>>
    struct search_result final
    {
        using value_type = typename Vector_::value_type;
        using iterator = typename Vector_::iterator;
        using const_iterator = typename Vector_::const_iterator;

        Vector_ data;
        std::mutex mutex;

        iterator begin() noexcept { return data.begin(); }
        iterator end() noexcept { return data.end(); }
        const_iterator begin() const noexcept { return data.begin(); }
        const_iterator end() const noexcept { return data.end(); }

        search_result() noexcept { data.reserve(512); }

        template <typename Tp_>
        void operator()(Tp_&& v, std::enable_if_t<std::is_same_v<std::remove_cvref_t<Tp_>, rwdata>, int> enableIf = 0)
        {
            if (v.size != sizeof(Tp)) [[unlikely]] {
                throw std::bad_cast();
            }

            std::lock_guard lock(mutex);
            data.push_back(*reinterpret_cast<Tp*>(v.data));
        }

        template <typename Tp_>
        void operator()(Tp_&& v, std::enable_if_t<std::is_same_v<std::remove_cvref_t<Tp_>, redata<Tp>>, int> enableIf = 0)
        {
            std::lock_guard lock(mutex);
            data.push_back(*reinterpret_cast<Tp*>(v.value));
        }
    };
}