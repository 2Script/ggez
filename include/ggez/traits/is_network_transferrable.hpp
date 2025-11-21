#pragma once
#include <type_traits>

#include "ggez/traits/is_implicitly_creatable_by_copy.hpp"


namespace ggez::traits {
    template<typename T>
    struct is_network_transferrable : std::conjunction<std::has_unique_object_representations<T>, is_implicitly_creatable_by_copy<T>> {};
}


namespace ggez::traits {
    template<typename T>
    constexpr bool is_network_transferrable_v = is_network_transferrable<T>::value;

    template<typename T>
    concept network_transferrable = is_network_transferrable_v<T>;
}