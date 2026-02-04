#pragma once

#include <string_view>
#include <vector>

#include "amemory/meta/to_string_view.hpp"
#include "amemory/detail/read_file.h"

#include "type_traits.hpp"

namespace amemory::detail
{
    template <parse_concept Parse, data_concept Data>
    class maps_impl
    {
        Data data_;
        mutable Parse parser_;
    public:
        using value_type = typename Parse::value_type;
        class iterator;

        maps_impl() = default;

        void file_load_data(std::string_view path)
        {
            auto size = detail::read_file(path.data(), data_.get_data(), data_.max_size());
            parser_.content(meta::to_string_view(data_.get_data(), size));
        }

        value_type next()
        {
            auto res = parser_();
            parser_.next_line();
            return res;
        }

        bool empty()
        {
            return parser_.max_pos() <= parser_.curr_pos();
        }

        iterator begin() const noexcept
        {
            return iterator(&parser_);
        }

        iterator end() const noexcept
        {
            return iterator(nullptr);
        }

        class iterator
        {
            Parse* self_;
        public:
            explicit iterator(Parse* self) : self_(self) {}

            iterator& operator++() noexcept
            {
                self_->next_line();
                return *this;
            }

            value_type operator*() noexcept
            {
                return (*self_)();
            }

            bool operator==(const iterator& rhs) const noexcept
            {
                return self_->max_pos() <= self_->curr_pos();
            }

            bool operator!=(const iterator& rhs) const noexcept
            {
                return !(*this == rhs);
            }

        };


    };
} // namespace amemory::detail
