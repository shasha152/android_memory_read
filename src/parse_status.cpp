//
// Created by 30980 on 2025/12/8.
//
#include "../include/amemory/detail/parse/parse_status.h"

#include <iostream>

#include "amemory/meta/algorithm_string.hpp"
#include "amemory/meta/to_string_view.hpp"

namespace amemory::detail
{
    status_value::status_value(std::string_view str)
        : value(str)
    {
    }

    status_value::status_value(std::int64_t num)
        : value(num)
    {
    }

    bool status_value::is_integer() const noexcept
    {
        return std::holds_alternative<std::int64_t>(value);
    }

    bool status_value::is_string() const noexcept
    {
        return std::holds_alternative<std::string_view>(value);
    }

    std::int64_t status_value::as_integer() const noexcept
    {
        return std::get<std::int64_t>(value);
    }

    std::string_view status_value::as_string() const noexcept
    {
        return std::get<std::string_view>(value);
    }


    parse_status::value_type parse_status::operator()()
    {
        return parse_line();
    }

    parse_status::value_type parse_status::parse_line()
    {
        value_type value;
        const auto key = next_token();
        value.first = key.substr(0, key.size() - 1);
        const auto content_ = content();
        while (content_[curr_pos()] != '\n' && curr_pos() < max_pos())
        {
            auto token = next_token();
            if (std::int64_t num = meta::to_int64(token); num != -1)
                value.second.emplace_back(num);
            else
                value.second.emplace_back(token);
        }

        return value;
    }
}
