#pragma once
#include <array>
#include <cstddef>
#include <cstring>
#include <mutex>
#include <shared_mutex>

#include "ggez/core/client_id_t.hpp"
#include "ggez/traits/is_network_transferrable.hpp"


namespace ggez {
    template<traits::network_transferrable StepT, std::size_t N>
    struct step_info {
        StepT step;
        std::array<bool, N> clients_on_step;

    public:
        void update(std::array<StepT, N> current_steps, client_id_t self_client_id, std::unique_lock<std::shared_mutex>& lock) noexcept {
            constexpr static std::array<bool, N> all_falses{};

            if(std::memcmp(clients_on_step.data(), all_falses.data(), N * sizeof(bool)) != 0)
                return;

            const std::size_t next_oldest_step = static_cast<std::size_t>(step + 1);
            lock.unlock();

            std::array<bool, N> players_on_oldest_step;

            for(std::size_t s = next_oldest_step; s <= current_steps[self_client_id]; ++s) {
                for(std::size_t i = 0; i < N; ++i)
                    players_on_oldest_step[i] = (current_steps[i] <= s);

                if(std::memcmp(players_on_oldest_step.data(), all_falses.data(), N * sizeof(bool)) != 0) {
                    lock.lock();
                    *this = {static_cast<StepT>(s), players_on_oldest_step};
                    return;
                }
            }

            __builtin_unreachable();
        }
    };
}