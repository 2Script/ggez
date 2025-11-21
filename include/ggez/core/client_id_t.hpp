#pragma once
#include <enet/enet.h>

namespace ggez {
    using client_id_t = enet_uint16;
}

namespace ggez {
    constexpr static client_id_t no_client_id = static_cast<client_id_t>(-1);
}