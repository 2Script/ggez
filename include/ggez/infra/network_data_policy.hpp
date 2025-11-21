#pragma once
#include <bit>
#include <bitset>
#include <cstdint>

#include <enet/enet.h>


namespace ggez {
    namespace network_data_policy_flag {
    enum : std::uint8_t {
        reliable            = ENET_PACKET_FLAG_RELIABLE,
        unsequenced         = ENET_PACKET_FLAG_UNSEQUENCED,
        manual_allocation   = ENET_PACKET_FLAG_NO_ALLOCATE,
        unreliable_fragment = ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT,
    };
    }
}


namespace ggez {
    using network_data_policy = std::uint8_t;
}
