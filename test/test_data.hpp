#pragma once
#include <array>

#include "./session_types.hpp"

namespace ggez::test {
    constexpr std::array<per_step_data_t, 2> per_step_data{{{0x1000, 0x07FF}, {0x007F, 0x2000}}};
}