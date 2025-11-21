#pragma once 
#include <type_traits>


namespace ggez::traits {
    template<typename T>
    struct is_implicit_lifetime : std::disjunction<
        std::is_scalar<T>,
        std::conjunction<
            std::is_aggregate<T>,
            std::is_trivially_destructible<T> //not 100% accurate (too strict), but this is as close as we can get since there is no trait for "not user-provided destructor"
        >,
        std::conjunction<
            std::is_trivially_destructible<T>,
            std::disjunction<
                std::is_trivially_default_constructible<T>,               
                std::is_trivially_copy_constructible<T>,
                std::is_trivially_move_constructible<T>
            >
        >,
        std::is_array<T>
    > {};
}

namespace ggez::traits {
    template<typename T>
    constexpr bool is_implicit_lifetime_v = is_implicit_lifetime<T>::value;

    template<typename T>
    concept implicit_lifetime = is_implicit_lifetime_v<T>;
}