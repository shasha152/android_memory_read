#pragma once

#include <string_view>
#include <concepts>

namespace amemory::detail
{
    class parse_base;

    template <typename Parse>
    concept parse_concept = std::is_base_of_v<parse_base, Parse>;

    template <typename Data>
    concept data_concept = requires(Data data) {
        { meta::to_string_view(data.get_data(), 1) } -> std::same_as<std::string_view>;
    } || requires(Data data) {
        { meta::to_string_view(data.get_data(), data.get_data().size()) } -> std::same_as<std::string_view>;
    };
}