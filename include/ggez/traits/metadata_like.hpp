#pragma once 
#include <type_traits>

#include "ggez/traits/is_network_transferrable.hpp"


namespace ggez::traits {
    template<typename T>
    concept metadata_like = network_transferrable<T> && true;
}