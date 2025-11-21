#pragma once
#include "ggez/session/basic_base.hpp"
#include <memory>

#include <enet/enet.h>

#include "ggez/infra/client_model.hpp"
#include "ggez/infra/ip_address.hpp"
#include "ggez/traits/metadata_like.hpp"
#include "ggez/traits/config_like.hpp"


namespace ggez::session::impl {
    template<traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT> requires traits::config_like<decltype(ConfigV)>
    constexpr     basic_base<PerStepDataT, ConfigV, MetadataT, StepT>::
    basic_base(MetadataT metadata, network_address_view addr, network_port port) noexcept :
        handle(), current_event{},
        session_metadata(metadata),
        host_net_addr(addr), host_net_port(port),
        enet_instance_tracker() {}
}


namespace ggez::session::impl {
    template<traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT> requires traits::config_like<decltype(ConfigV)>
    constexpr int     basic_base<PerStepDataT, ConfigV, MetadataT, StepT>::
    active(timeout_ms_t timeout_ms) noexcept {
        return enet_host_service(handle.get(), &current_event, timeout_ms);
    }
}

namespace ggez::session::impl {
    template<traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT> requires traits::config_like<decltype(ConfigV)>
    template<typename F, typename... Args>
    std::invoke_result_t<F, Args...>     basic_base<PerStepDataT, ConfigV, MetadataT, StepT>::
    invoke_callback(F&& func, Args&&... args) noexcept {
        if(!func) return {};
        return std::invoke(std::forward<F>(func), std::forward<Args>(args)...);
    }
}