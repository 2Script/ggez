#pragma once
#include <cstddef>
#include <memory>

#include <enet/enet.h>

#include "ggez/aux/enet_host_deleter.hpp"
#include "ggez/infra/network_address_view.hpp"
#include "ggez/infra/network_stats_t.hpp"
#include "ggez/infra/network_port.hpp"
#include "ggez/session/config_t.hpp"
#include "ggez/session/metadata_t.hpp"
#include "ggez/aux/instance_tracker.hpp"
#include "ggez/session/timeout_ms_t.hpp"
#include "ggez/traits/has_valid_client_count_upper_limits.hpp"
#include "ggez/traits/metadata_like.hpp"
#include "ggez/traits/config_like.hpp"


namespace ggez::session::impl {
    template<traits::network_transferrable PerStepDataT, auto ConfigV = session::config_t{}, traits::metadata_like MetadataT = session::metadata_t<ConfigV>, traits::network_transferrable StepT = std::size_t> requires traits::config_like<decltype(ConfigV)>
    class basic_base {
        static_assert(traits::has_valid_client_count_upper_limits_v<ConfigV>);
    public:
        constexpr basic_base() noexcept = default;
        constexpr basic_base(MetadataT metadata, network_address_view addr, network_port port) noexcept;

    public:
        constexpr int active(timeout_ms_t timeout_ms = default_timeout_ms) noexcept;

    public:
        consteval static decltype(ConfigV) config() noexcept { return ConfigV; }

        constexpr MetadataT const& metadata() const noexcept { return this->session_metadata; }
        constexpr MetadataT      & metadata()       noexcept { return this->session_metadata; }

        constexpr network_stats_t stats() const noexcept;

    protected:
        template<typename F, typename... Args>
        static std::invoke_result_t<F, Args...> invoke_callback(F&& func, Args&&... args) noexcept;

    protected:
        std::unique_ptr<ENetHost, ::ggez::impl::enet_host_deleter> handle;
        ENetEvent current_event;

        MetadataT session_metadata;

        network_address_view host_net_addr;
        network_port host_net_port;
        
        [[no_unique_address]] ::ggez::impl::instance_tracker enet_instance_tracker;

    protected:
        constexpr static std::size_t metadata_channel_id = 0;
    };
}

#include "ggez/session/basic_base.inl"