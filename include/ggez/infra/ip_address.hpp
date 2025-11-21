#pragma once
#include <array>
#include <cstdint>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#else 
#include <arpa/inet.h>
#endif

#include "ggez/infra/network_address_view.hpp"


#if __has_builtin (__builtin_char_memchr)
#define __ggez_memchr __builtin_char_memchr
#define __ggez_memchr_constexpr constexpr
#else
#define __ggez_memchr __builtin_memchr
#define __ggez_memchr_constexpr inline
#endif


namespace ggez {
    struct ip_address : std::array<char, INET_ADDRSTRLEN> {
        constexpr static std::size_t max_size = INET_ADDRSTRLEN - 1;
    public:
        __ggez_memchr_constexpr std::size_t size() const noexcept;

        constexpr operator network_address_view() const noexcept;
    };
}


namespace ggez {
    using ip_address_bytes_t = std::array<std::uint8_t, 4>;
    using ip_address_uint_t = in_addr;

    constexpr ip_address localhost{"127.0.0.1"};
    constexpr ip_address any_host{"0.0.0.0"};
}


namespace ggez {
    constexpr ip_address to_ip_address(ip_address_bytes_t bytes) noexcept;
    constexpr ip_address to_ip_address(ip_address_uint_t val) noexcept;
}

#include "ggez/infra/ip_address.inl"
