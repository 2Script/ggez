#pragma once
#include <cstddef>
#include <new>

#include "ggez/traits/is_implicit_lifetime.hpp"


namespace ggez {
    template<::ggez::traits::implicit_lifetime T>
    T* start_lifetime_as(void* p) noexcept {
        std::byte* bytes = new(p) std::byte[sizeof(T)];
        return std::launder(reinterpret_cast<T*>(bytes));
    }

    template<::ggez::traits::implicit_lifetime T>
    T const* start_lifetime_as(void const* p) noexcept {
        void* mutable_p = const_cast<void*>(p);
        std::byte const* bytes = new(mutable_p) std::byte[sizeof(T)];
        return std::launder(reinterpret_cast<T const*>(bytes));
    }
}