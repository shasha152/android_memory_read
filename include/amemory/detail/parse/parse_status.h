#pragma once

#include <string_view>
#include <variant>
#include <vector>

#include "parse_base.h"

namespace amemory::detail
{
    struct status_value
    {
        std::variant<std::int64_t, std::string_view> value;

        explicit status_value(std::string_view);
        explicit status_value(std::int64_t);

        MEMORY_NODISCARD bool is_integer() const noexcept;
        MEMORY_NODISCARD bool is_string() const noexcept;

        MEMORY_NODISCARD std::int64_t as_integer() const noexcept;
        MEMORY_NODISCARD std::string_view as_string() const noexcept;
    };

    class parse_status : public parse_base
    {
    public:
        using value_type = std::pair<std::string_view, std::vector<status_value>>;

        parse_status() = default;
        explicit parse_status(std::string_view content) : parse_base(content) {}

        value_type operator()();

    private:
        value_type parse_line();
    };
}