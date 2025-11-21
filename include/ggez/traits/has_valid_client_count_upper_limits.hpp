#pragma once
#include <cstddef>
#include <type_traits>

#include "ggez/infra/client_counts_t.hpp"
#include "ggez/infra/client_model.hpp"

namespace ggez::traits {
    template<auto C>
    struct has_valid_client_count_upper_limits : std::bool_constant<
        C.client_count_upper_limits[static_cast<std::size_t>(client_model::player)] <= client_count_upper_limits_max[static_cast<std::size_t>(client_model::player)] &&
        C.client_count_upper_limits[static_cast<std::size_t>(client_model::spectator)] <= client_count_upper_limits_max[static_cast<std::size_t>(client_model::spectator)] &&
        C.client_count_upper_limits[static_cast<std::size_t>(client_model::player)] + C.client_count_upper_limits[static_cast<std::size_t>(client_model::spectator)] <= combined_client_count_upper_limit_max
    > {};
}

namespace ggez::traits {
    template<auto C>
    constexpr bool has_valid_client_count_upper_limits_v = has_valid_client_count_upper_limits<C>::value;
}