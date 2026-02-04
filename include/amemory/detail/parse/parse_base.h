#pragma once

#include <string_view>

#include "amemory/meta/algorithm_string.hpp"

#define MEMORY_NODISCARD [[nodiscard]]

namespace amemory::detail
{
    class parse_interface
    {
    public:
        virtual ~parse_interface() = default;
        MEMORY_NODISCARD virtual std::size_t curr_pos() const noexcept = 0;
        MEMORY_NODISCARD virtual std::size_t max_pos() const noexcept = 0;
    };

    struct skip_sw_token
    {
        bool operator()(char c) const noexcept
        {
            return c != ' ' && c != '\t' && c != '\n';
        }
    };

    class parse_base : public parse_interface
    {
        static constexpr auto DEFAULT_TOKEN_CALL = skip_sw_token{};
    protected:
        std::string_view content_;
        std::size_t current_pos_ = 0;
    public:
        parse_base() = default;
        explicit parse_base(std::string_view text);

        void content(std::string_view content) noexcept;
        MEMORY_NODISCARD std::string_view content() const noexcept;

        MEMORY_NODISCARD std::size_t max_pos() const noexcept override;
        MEMORY_NODISCARD std::size_t curr_pos() const noexcept override;

        template <typename CallBack>
        std::string_view next_token(CallBack&& call_back) noexcept;
        std::string_view next_token() noexcept;

        MEMORY_NODISCARD std::string_view get_token(std::size_t n) noexcept;

        void next_line() noexcept;
    };

    template <typename CallBack>
    inline std::string_view parse_base::next_token(CallBack&& call_back) noexcept
    {
        meta::skip_whitespace(content_, current_pos_);
        std::size_t pos = current_pos_;
        while (current_pos_ < content_.size())
        {
            char c = content_[current_pos_];
            if (!std::forward<CallBack>(call_back)(c))
                break;
            ++current_pos_;
        }
        return content_.substr(pos, current_pos_ - pos);
    }
}
