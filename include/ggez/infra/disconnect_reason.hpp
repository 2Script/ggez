#pragma once
#include <cstdint>


namespace ggez {
    using disconnect_reason_t = std::uint_fast8_t;
}

namespace ggez {
    namespace disconnect_reason {
    enum : disconnect_reason_t {
        timed_out,
        requested_remotely,
        requested_locally,
        forced,

        num_reasons,
    };
    }
}


namespace ggez {
    constexpr static std::size_t num_disconnect_reasons = disconnect_reason::num_reasons;
}