#pragma once
#include <type_traits>

#include "ggez/infra/network_data_id.hpp"


namespace ggez {

    template<typename FirstT, typename SecondT = network_data_id_t>
    struct network_data_offset : std::integral_constant<std::size_t, (sizeof(FirstT) + alignof(SecondT) - 1) / alignof(SecondT) * alignof(SecondT)> {};
}

namespace ggez {
    template<typename FirstT, typename SecondT = network_data_id_t>
    constexpr bool network_data_offset_v = network_data_offset<FirstT, SecondT>::value;
}