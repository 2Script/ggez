#pragma once

#include <enet/enet.h>


namespace ggez {
    using network_data_id_t = enet_uint8;
}

namespace ggez {
    namespace network_data_id {
    enum : network_data_id_t {
        none,
        metadata,

        client_id,
        config,
        confirmation,

        num_default_ids,

        per_step_data = none,
    };
    };
}


namespace ggez {
    constexpr static std::size_t num_default_network_data_ids = network_data_id::num_default_ids;
}