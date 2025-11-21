#pragma once
#include <new>

#include <enet/enet.h>


namespace ggez {
    using endpoint = ENetPeer;
}


namespace ggez {
    template<typename InfoT>
    struct session_endpoint : endpoint {
        //TODO: constexpr?
        inline InfoT      & info()       noexcept { return *std::launder(reinterpret_cast<InfoT      *>(&this->data)); }
        inline InfoT const& info() const noexcept { return *std::launder(reinterpret_cast<InfoT const*>(&this->data)); }
    };
}
