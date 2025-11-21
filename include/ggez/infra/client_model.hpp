#pragma once
#include <cstddef>
#include <limits>
#include <array>
#include <string_view>
#include <string>
#include <type_traits>


namespace ggez {
    enum class client_model : bool {
        player,
        spectator
    };
}


namespace ggez {
    constexpr std::size_t num_client_models = std::numeric_limits<std::underlying_type_t<client_model>>::max() + 1;
}


namespace ggez::impl {
    constexpr std::array<std::string_view, num_client_models> client_model_names{"player", "spectator"};
}

namespace ggez {
    constexpr std::string_view to_string_view(client_model m) noexcept {
        return impl::client_model_names[static_cast<std::size_t>(m)];
    }
    
    constexpr std::string to_string(client_model m) noexcept {
        return std::string(impl::client_model_names[static_cast<std::size_t>(m)]);
    }
}

namespace ggez {
    template<typename TraitsT>
    std::basic_ostream<char, TraitsT>& operator<<(std::basic_ostream<char, TraitsT>& os, client_model m) noexcept {
        return os << to_string_view(m);
    }
}