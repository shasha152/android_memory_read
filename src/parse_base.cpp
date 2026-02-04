#include "amemory/detail/parse/parse_base.h"

#include "amemory/meta/algorithm_string.hpp"

namespace amemory::detail
{

    parse_base::parse_base(std::string_view text)
        : content_(text)
    {
    }

    void parse_base::content(std::string_view content) noexcept
    {
        current_pos_ = 0;
        content_ = content;
    }

    std::string_view parse_base::content() const noexcept
    {
        return content_;
    }


    std::size_t parse_base::max_pos() const noexcept
    {
        return content_.size();
    }

    std::size_t parse_base::curr_pos() const noexcept
    {
        return current_pos_;
    }

    std::string_view parse_base::next_token() noexcept
    {
        return next_token(DEFAULT_TOKEN_CALL);
    }

    std::string_view parse_base::get_token(std::size_t n) noexcept
    {
        meta::skip_whitespace(content_, current_pos_);
        const std::size_t pos = current_pos_;

        const std::size_t size = content_.size();
        const std::size_t remain = size - pos;
        const std::size_t take = std::min(remain, n);

        current_pos_ += take;

        return content_.substr(pos, take);
    }

    void parse_base::next_line() noexcept
    {
        if (curr_pos() < max_pos())
        {
            std::size_t p = content_.find('\n', curr_pos());
            current_pos_ = p + 1;
        }
    }
}
