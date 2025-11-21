#pragma once
#include <array>
#include <concepts>
#include <cstdint>
#include <type_traits>


namespace ggez {
    template<std::integral T>
    constexpr T byte_swap(T val) noexcept {
        static_assert(std::has_unique_object_representations_v<T>, "Value type for integral byte-swap must not have any padding bits");
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


namespace ggez {
    template<typename T> requires std::is_enum_v<T>
    constexpr T byte_swap(T val) noexcept {
        return static_cast<T>(byte_swap(static_cast<std::underlying_type_t<T>>(val)));
    }

    
    template<typename T, std::size_t N>
    constexpr std::array<T, N> byte_swap(std::array<T, N> const& val) noexcept {

        std::array<T, N> ret;
        for(std::size_t i = 0; i < N; ++i)
            ret[i] = byte_swap(val[i]);
        return ret;
    }
}