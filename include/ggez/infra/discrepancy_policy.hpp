#pragma once
#include <cstdint>


namespace ggez {
    enum class discrepancy_policy : std::uint_least8_t {
        ignore,
        delay,
        rollback,
    };
}