#pragma once
#include <cstddef>
#include <cstring>

#include "ggez/traits/is_implicit_lifetime.hpp"


namespace ggez {
    template<traits::implicit_lifetime T>
    T from_bytes(void const* p) noexcept {
        alignas(T) std::byte bytes[sizeof(T)];
        void const* bytes_ptr = std::memcpy(&bytes, p, sizeof(T));
        return *reinterpret_cast<T const*>(bytes_ptr);
    }
}