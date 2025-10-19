#pragma once
#include <bit>
#include <concepts>
#include <cstdint>
#include <type_traits>


namespace ggez::byte_order::impl {
    template<std::integral T>
    constexpr T byte_swap(T val) noexcept {
        static_assert(std::has_unique_object_representations_v<T>, "Value type for byte-swap must not have any padding bits");
        if constexpr (sizeof(T) == 1)
            return val;
        else if constexpr(sizeof(T) == 2)
            return static_cast<T>(__builtin_bswap16(static_cast<std::uint16_t>(val)));
        else if constexpr(sizeof(T) == 4)
            return static_cast<T>(__builtin_bswap32(static_cast<std::uint32_t>(val)));
        else if constexpr(sizeof(T) == 8)
            return static_cast<T>(__builtin_bswap64(static_cast<std::uint64_t>(val)));
        else
            static_assert(!std::is_same_v<T, T>, "Unsupported integral size for byte swap");
    }
}

namespace ggez::byte_order {
    template<std::integral T>
    constexpr T host_to_network(T val) noexcept {
        static_assert(std::endian::native == std::endian::little || std::endian::native == std::endian::big, "Mixed-endianness is not supported");
        if constexpr (std::endian::native == std::endian::big) return val;
        else return impl::byte_swap(val);
    }

    template<std::integral T>
    constexpr T network_to_host(T val) noexcept {
        static_assert(std::endian::native == std::endian::little || std::endian::native == std::endian::big, "Mixed-endianness is not supported");
        if constexpr (std::endian::native == std::endian::big) return val;
        else return impl::byte_swap(val);
    }
}
