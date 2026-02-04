#pragma once

#include "get_offset.hpp"
#include "amemory/detail/pfr/for_each.hpp"
#include "amemory/detail/mem/pfr/get_offset.hpp"
#include "amemory/detail/mem/pfr/get_value.hpp"
#include "amemory/detail/mem/pfr/layout_validator.hpp"

#include "amemory/detail/mem/vm_readwrite.h"

namespace amemory::detail
{
    template <typename T, typename ArrayLocal, typename ArrayRemote, std::size_t Index = 0,
        typename = void>
    struct fill_array
    {
        static constexpr std::size_t fill(
            T& value, std::uintptr_t addr,
            ArrayLocal& local, ArrayRemote& remote, int start) noexcept
        {
            if constexpr (Index < boost::pfr::tuple_size_v<T>)
            {
                const std::size_t real_index  = start + Index;
                constexpr std::size_t _field_size  = field_size<T, Index>::size;
                constexpr std::size_t _field_offset = get_offset<T, Index>::value;

                local[real_index].size  = _field_size;
                remote[real_index].size = _field_size;

                local[real_index].address =
                    std::addressof(get_value(boost::pfr::get<Index>(value)));
                remote[real_index].address = addr + _field_offset;

                return fill_array<T, ArrayLocal, ArrayRemote, Index + 1>::fill(
                                  value, addr, local, remote, start);
            }
            else
            {
                return boost::pfr::tuple_size_v<T> + start;
            }
        }
    };

    template <typename T, typename ArrayLocal, typename ArrayRemote, std::size_t Index>
    struct fill_array<T, ArrayLocal, ArrayRemote, Index,
        std::enable_if_t<!std::is_class_v<T>>>
    {
        static constexpr std::size_t fill(
            T& value, std::uintptr_t addr,
            ArrayLocal& local, ArrayRemote& remote, int start) noexcept
        {
            const std::size_t real_index  = start + Index;

            local[real_index].size  = sizeof(T);
            remote[real_index].size = sizeof(T);

            local[real_index].address =
                    std::addressof(value);
            remote[real_index].address = addr;

            return start + 1;
        }
    };

    template <typename T, typename = void>
    struct field_number : std::integral_constant<std::size_t, 1> {};

    template <typename T>
    struct field_number<T, std::enable_if_t<
            is_pfr_aggregate_v<T>
        >> : std::integral_constant<std::size_t, boost::pfr::tuple_size_v<T>> {};

    template <typename T, std::size_t TI, std::size_t RI, std::size_t S>
    consteval std::size_t array_index_start()
    {
        constexpr std::size_t N = boost::pfr::tuple_size_v<T>;

        static_assert(TI < N, "TI out of range");
        if constexpr (RI == TI)
        {
            return S;
        }
        else
        {
            using field_t = boost::pfr::tuple_element_t<RI, T>;

            if constexpr (is_offset_pointer_v<field_t>)
            {
                return array_index_start<
                    T, TI, RI + 1,
                    S + field_number<typename field_t::value_type>::value>();
            }
            else
            {
                return array_index_start<T, TI, RI + 1, S>();
            }
        }
    }




    template <typename Mem>
    class manager
    {
        Mem& mem;
    public:
        explicit manager(Mem& mem) noexcept : mem(mem) {}

        template <typename T>
        void read(std::uintptr_t addr, T& value)
        {
            constexpr auto max_size = max_field_count_v<T>;
            // std::cout << max_size << std::endl;
            using local_array_t = std::array<local_vm_region, max_size>;
            using remote_array_t = std::array<remote_vm_region, max_size>;
            local_array_t local;
            remote_array_t remote;

            layout_validator<T>::check();
            const auto size = fill_array<T, local_array_t, remote_array_t, 0>::fill(value, addr, local, remote, 0);
            mem.vm_read(local.data(), remote.data(), size);
            read_level<local_array_t, remote_array_t>(*this, local, remote).
                read(value);
        }
    private:
        template <typename ArrayLocal, typename ArrayRemote>
        class read_level
        {
            ArrayLocal& local_;
            ArrayRemote& remote_;
            manager &self;
            int count = 0;
        public:
            constexpr explicit read_level(manager &self, ArrayLocal& local, ArrayRemote& remote) noexcept :
                local_(local), remote_(remote), self(self) {}

            template <typename T>
            void read(T& value)
            {
                constexpr auto depth = max_pointer_depth_v<T>;

                [&]<std::size_t... I>(std::index_sequence<I...>)
                {
                    (read_pointer<T, I, depth>(value), ...);
                }(std::make_index_sequence<depth + 1>{});
            }
        private:
            template <typename T, std::size_t I, std::size_t D>
            void read_pointer(T& value)
            {
                if constexpr (I != 0)
                {
                    count = 0;
                    max_field_count_proxy_<T, D + 1, 0, I>(value, 0, count);
                    self.mem.vm_read(local_.data(), remote_.data(), count);
                }

            }


            template <typename Ty, std::size_t D, std::size_t N, std::size_t T>
            void max_field_count_proxy_(Ty& value, std::uintptr_t addr, int& count) noexcept
            {
                if constexpr (D != N)
                {
                    if constexpr (N == T)
                    {

                        count = fill_array<Ty, ArrayLocal, ArrayRemote>::fill(value, addr, local_, remote_, count);
                    }
                    if constexpr (is_pfr_aggregate<Ty>::value)
                    {
                        max_field_pos_array_<Ty, D, N + 1, T>(value, count);
                    }

                }
            }

            template <typename Ty, std::size_t D, std::size_t N, std::size_t T>
            void max_field_pos_array_(Ty& value, int& count)
            {
                [&]<std::size_t... I>(index_list<I...>) {
                    (max_field_count_proxy_<
                        typename boost::pfr::tuple_element_t<I, Ty>::value_type, D, N, T>(*boost::pfr::get<I>(value),
                            boost::pfr::get<I>(value).get(), count), ...);
                }(pointer_list_t<Ty>{});
            }
        };
    };


}