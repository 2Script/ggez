#pragma once
#include <cstdint>
#include <limits>

#include <enet/enet.h>
#include <type_traits>

#include "ggez/core/client_id_t.hpp"
#include "ggez/infra/client_model.hpp"
#include "ggez/infra/disconnect_reason.hpp"


namespace ggez {
    template<std::size_t MaxNumPlayers>
    struct alignas(void*) client_info {
        constexpr client_info() = default;
        constexpr client_info(client_id_t id, disconnect_reason_t reason = disconnect_reason::timed_out) noexcept :
            __info_bits(((static_cast<std::uintptr_t>(id) & std::numeric_limits<client_id_t>::max()) << 0) | ((static_cast<std::uintptr_t>(reason) & 0b111) << std::numeric_limits<client_id_t>::digits)) {}

    public:
        constexpr client_id_t id() const noexcept {
            return static_cast<client_id_t>((__info_bits >> 0) & std::numeric_limits<client_id_t>::max());
        }

        constexpr client_model model() const noexcept {
            return static_cast<client_model>(id() >= MaxNumPlayers);
        }
        
        constexpr disconnect_reason_t disconn_reason() const noexcept {
            return static_cast<disconnect_reason_t>((__info_bits >> std::numeric_limits<client_id_t>::digits) & 0b111);
        }
        
    private:
        std::uintptr_t __info_bits;
    };
}