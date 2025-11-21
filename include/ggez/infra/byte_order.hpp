#pragma once
#include <bit>
#include <type_traits>

#include "ggez/aux/byte_swap.hpp"


namespace ggez::byte_order {
    template<typename T>
    constexpr std::remove_cvref_t<T> host_to_network(T&& val) noexcept {
        static_assert(std::endian::native == std::endian::little || std::endian::native == std::endian::big, "Mixed-endianness is not supported");
        using ::ggez::byte_swap;

        if constexpr (std::endian::native == std::endian::big) return std::forward<T>(val);
        else return byte_swap(std::forward<T>(val));
    }

    template<typename T>
    constexpr std::remove_cvref_t<T> network_to_host(T&& val) noexcept {
        static_assert(std::endian::native == std::endian::little || std::endian::native == std::endian::big, "Mixed-endianness is not supported");
        using ::ggez::byte_swap;

        if constexpr (std::endian::native == std::endian::big) return std::forward<T>(val);
        else return byte_swap(std::forward<T>(val));
    }
}
