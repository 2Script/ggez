#pragma once
#include <cstdint>
#include <limits>

#include <enet/enet.h>
#include <type_traits>

#include "ggez/core/client_id_t.hpp"
#include "ggez/infra/client_model.hpp"
#include "ggez/infra/disconnect_reason.hpp"


namespace ggez {
    struct alignas(void*) host_info {
        constexpr host_info() = default;
        constexpr host_info(disconnect_reason_t reason) noexcept :
            __info_bits(((static_cast<std::uintptr_t>(reason) & 0b11) << 0)) {}

    public:
        constexpr disconnect_reason_t disconn_reason() const noexcept {
            return static_cast<disconnect_reason_t>((__info_bits >> 0) & 0b11);
        }
        
    private:
        std::uintptr_t __info_bits;
    };
}