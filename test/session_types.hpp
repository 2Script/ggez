#pragma once
#include <cstddef>
#include <cstdint>

#include "ggez/aux/byte_swap.hpp"
#include "ggez/session/basic_client.hpp"
#include "ggez/session/basic_host.hpp"
#include "ggez/session/config_t.hpp"
#include "ggez/session/metadata_t.hpp"


namespace ggez::test {
    struct per_step_data_t {
        std::uint16_t hi;
        std::uint16_t lo;
    };

    constexpr session::config_t config{discrepancy_policy::ignore};

    using metadata_t = session::metadata_t<config>;
    using step_t     = std::size_t;
}

namespace ggez::test {
    using client = basic_client_session<per_step_data_t, config, metadata_t, step_t>;
    using host   = basic_host_session<  per_step_data_t, config, metadata_t, step_t>;

    constexpr network_port port = 6969;
}


namespace ggez::test {
    constexpr per_step_data_t byte_swap(per_step_data_t val) noexcept {
        using ggez::byte_swap;
        return {byte_swap(val.hi), byte_swap(val.lo)};
    }
}