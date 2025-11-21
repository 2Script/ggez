#pragma once
#include <bitset>
#include <cstddef>

#include "ggez/core/client_id_t.hpp"
#include "ggez/infra/client_model.hpp"
#include "ggez/session/config_t.hpp"


namespace ggez {
    template<std::size_t MaxNumPlayers, std::size_t MaxNumSpectators>
    struct client_id_set : std::bitset<(MaxNumPlayers + MaxNumSpectators)> {
    public:
        constexpr std::bitset<MaxNumPlayers> player_ids() const noexcept { 
            return std::bitset<MaxNumPlayers>(this->to_string());
        }

        constexpr std::bitset<MaxNumSpectators> spectator_ids() const noexcept {
            return std::bitset<MaxNumSpectators>(this->to_string(), MaxNumPlayers);
        }

        constexpr std::bitset<(MaxNumPlayers + MaxNumSpectators)> client_ids() const noexcept {
            return *this;
        }
        
    public:
        constexpr client_id_t count_players() const noexcept {
            return (*this << MaxNumSpectators).count();
        }

        constexpr client_id_t count_spectators() const noexcept {
            return (*this >> MaxNumPlayers).count();
        }

        constexpr client_counts_t count_clients() const noexcept {
            return {count_players(), count_spectators()};
        }
        
    public:
        constexpr client_id_t find_first_free_player_id() const noexcept { 
            return std::min((~(*this))._Find_first(), MaxNumPlayers);
        }
        
        constexpr client_id_t find_first_free_spectator_id() const noexcept { 
            return (~(*this))._Find_next(MaxNumPlayers - 1) - MaxNumSpectators;
        }

        constexpr client_id_t find_first_free_client_id(client_model model) const noexcept { 
            return (model == client_model::player) ? find_first_free_player_id() : find_first_free_spectator_id();
        }
    };
}