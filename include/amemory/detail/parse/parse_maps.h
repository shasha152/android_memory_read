#pragma once

#include <string_view>
#include <vector>

#include "parse_base.h"
#include "map_entry.h"
#include "amemory/meta/memory_t.h"

namespace amemory::detail
{

    class parse_maps : public parse_base
    {
    public:
        using value_type = map_entry;

        parse_maps() = default;
        explicit parse_maps(std::string_view maps_content);

        value_type operator()();

    private:
        value_type parse_line();
        void skip_whitespace();
    };
}
