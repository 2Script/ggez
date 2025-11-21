#pragma once
#include <limits>

#include <enet/enet.h>


namespace ggez::session {
    using timeout_ms_t = enet_uint32;
}


namespace ggez::session {
    constexpr timeout_ms_t no_timeout = static_cast<timeout_ms_t>(-1);

    constexpr timeout_ms_t default_timeout_ms = 5000;
}