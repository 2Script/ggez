#pragma once
#include <type_traits>

#include "ggez/traits/is_implicit_lifetime.hpp"


namespace ggez::traits {
    template<typename T>
    struct is_implicitly_creatable_by_copy : std::conjunction<std::is_trivially_copyable<T>, is_implicit_lifetime<T>> {};
}


namespace ggez::traits {
    template<typename T>
    constexpr bool is_implicitly_creatable_by_copy_v = is_implicitly_creatable_by_copy<T>::value;

    template<typename T>
    concept implicitly_creatable_by_copy = is_implicitly_creatable_by_copy_v<T>;
}