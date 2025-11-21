#pragma once
#include <enet/enet.h>

namespace ggez::impl {
    struct enet_host_deleter {
        constexpr void operator()(ENetHost* ptr) noexcept {
            return enet_host_destroy(ptr);
        }
    };
}
