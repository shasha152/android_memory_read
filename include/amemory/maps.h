#pragma once

#include <array>

#include "amemory/detail/base.h"
#include "amemory/detail/parse/parse_maps.h"
#include "amemory/detail/impl/maps_impl.hpp"
#include "amemory/meta/memory_t.h"

namespace amemory
{

    template <typename Data = detail::data<1024 * 512>, typename Parse = detail::parse_maps>
    class basic_maps : public detail::base
    {
        using impl = detail::maps_impl<Parse, Data>;

        impl impl_;
    public:
        using value_type = typename impl::value_type;
        using iterator = typename impl::iterator;

        explicit basic_maps(pid_t pid) : base(pid) {}

        void load()
        {
            impl_.file_load_data(std::string(path()) + "/maps");
        }

        value_type next()
        {
            return impl_.next();
        }

        bool empty()
        {
            return impl_.empty();
        }

        iterator begin() const noexcept
        {
            return impl_.begin();
        }

        iterator end() const noexcept
        {
            return impl_.end();
        }
    };

    using maps = basic_maps<>;
    using map_entry = detail::map_entry;
    using memory_t = meta::memory_t;

    template <typename Parse, typename Data>
    std::vector<
        typename basic_maps<Data, Parse>::value_type>
    to_vector(const basic_maps<Data, Parse>& maps_) noexcept
    {
        std::vector<typename basic_maps<Data, Parse>::value_type> result;
        result.reserve(1024);

        for (auto m : maps_) result.push_back(m);
        return result;
    }

    template <typename Parse = detail::memory_type_parse>
    memory_t to_memory_type(const map_entry& m, Parse&& parse = detail::memory_type_parse{}) noexcept
    {
        return std::forward<Parse>(parse)(m);
    }

    inline std::string_view to_string(memory_t memory) noexcept
    {
        return meta::to_string(memory);
    }
}
