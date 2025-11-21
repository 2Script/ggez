#pragma once
#include <cstddef>

#include "ggez/aux/byte_swap.hpp"
#include "ggez/aux/client_id_set.hpp"
#include "ggez/infra/client_model.hpp"
#include "ggez/session/config_t.hpp"
#include "ggez/traits/config_like.hpp"


namespace ggez::session {
    template<auto ConfigV> requires traits::config_like<decltype(ConfigV)>
    struct metadata_t {
        constexpr static std::size_t max_num_players = ConfigV.client_count_upper_limits[static_cast<std::size_t>(client_model::player)];
        constexpr static std::size_t max_num_spectators = ConfigV.client_count_upper_limits[static_cast<std::size_t>(client_model::spectator)];
        constexpr static std::size_t max_num_clients = max_num_players + max_num_spectators;

    public:
        std::size_t fixed_step_delay;

        client_id_t min_player_count{2};
        client_counts_t max_client_counts{max_num_players, max_num_spectators};

        std::array<std::byte, alignof(client_id_set<max_num_players, max_num_spectators>) - (sizeof(client_counts_t) + sizeof(client_id_t))> __padding2{};


        client_id_set<max_num_players, max_num_spectators> active_client_ids{};
    };
}

namespace ggez::session {
    template<auto ConfigV> requires traits::config_like<decltype(ConfigV)>
    constexpr metadata_t<ConfigV> byte_swap(metadata_t<ConfigV> val) noexcept {
        using ::ggez::byte_swap;

        return {
            byte_swap(val.fixed_step_delay),
            byte_swap(val.min_player_count),
            byte_swap(val.max_client_counts),
            val.__padding2,
            val.active_client_ids, //TODO: swap or use 8-bit long word for bitset
        };
    }
}
