#pragma once
#include "ggez/infra/ip_address.hpp"


namespace ggez {
    constexpr std::size_t ip_address::size() const noexcept {
        char const* termination = static_cast<char const*>(__ggez_memchr(data(), '\0', max_size + 1));
        return static_cast<std::size_t>(termination - data());
    }


    constexpr ip_address::operator network_address_view() const noexcept {
        return network_address_view{data(), size()};
    }
}


namespace ggez {
    constexpr ip_address to_ip_address(ip_address_bytes_t val) noexcept {
        return to_ip_address(std::bit_cast<ip_address_uint_t>(val));
    }

    constexpr ip_address to_ip_address(ip_address_uint_t val) noexcept {
        ip_address ret{};
        inet_ntop(AF_INET, &val, ret.data(), ip_address::max_size + 1);
        return ret;
    }
}
