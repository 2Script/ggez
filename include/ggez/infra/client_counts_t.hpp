#pragma once
#include <array>

#include <enet/enet.h>

#include "ggez/core/client_id_t.hpp"
#include "ggez/infra/client_model.hpp"



namespace ggez {
    using client_counts_t = std::array<client_id_t, num_client_models>;
}

namespace ggez {
    constexpr static client_id_t combined_client_count_upper_limit_max = ENET_PROTOCOL_MAXIMUM_PEER_ID + 1;
    constexpr static client_counts_t client_count_upper_limits_max{ENET_PROTOCOL_MAXIMUM_CHANNEL_COUNT - 1, combined_client_count_upper_limit_max};
}