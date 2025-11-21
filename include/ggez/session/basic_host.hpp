#pragma once
#include <cstddef>
#include <cstdint>

#include "ggez/infra/network_data_id.hpp"
#include "ggez/session/callbacks.hpp"
#include "ggez/infra/client_info.hpp"
#include "ggez/core/client_id_t.hpp"
#include "ggez/infra/client_model.hpp"
#include "ggez/infra/disconnect_reason.hpp"
#include "ggez/infra/endpoint.hpp"
#include "ggez/session/config_t.hpp"
#include "ggez/infra/network_address_view.hpp"
#include "ggez/infra/network_port.hpp"
#include "ggez/session/basic.hpp"
#include "ggez/session/basic_base.hpp"
#include "ggez/session/model.hpp"


namespace ggez::session {
    template<traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT> requires traits::config_like<decltype(ConfigV)>
    class basic<session::model::host, PerStepDataT, ConfigV, MetadataT, StepT> : public impl::basic_base<PerStepDataT, ConfigV, MetadataT, StepT> {
    public:
        using callbacks_type = session::callbacks<session::model::host, PerStepDataT, ConfigV, MetadataT, StepT>;

        constexpr static std::size_t max_num_players = ConfigV.client_count_upper_limits[static_cast<std::size_t>(client_model::player)];
        constexpr static std::size_t max_num_spectators = ConfigV.client_count_upper_limits[static_cast<std::size_t>(client_model::spectator)];
        constexpr static std::size_t max_num_clients = max_num_players + max_num_spectators;
        constexpr static std::size_t all_clients = static_cast<client_id_t>(-1);
        using client_info_type = client_info<max_num_players>;
        using client_endpoint = session_endpoint<client_info_type>;

    private:
        using base_type = impl::basic_base<PerStepDataT, ConfigV, MetadataT, StepT>;
    public:
        constexpr basic() noexcept = default;
        constexpr basic(network_address_view addr, network_port port, MetadataT initial_metadata, callbacks_type session_callbacks = {}) noexcept;
    public:
        result<void> initialize() noexcept;
        
    public:
        constexpr network_address_view const& address() const noexcept { return this->host_net_addr; }
        constexpr network_port    const& port()    const noexcept { return this->host_net_port; }

    public:
        result<void> relay_per_step_data(PerStepDataT data, StepT step, client_id_t data_source_client_id, client_id_t receipient_client_id = all_clients) noexcept;
        result<void> send_per_step_data(PerStepDataT data, StepT step, client_id_t data_source_client_id, client_id_t receipient_client_id = all_clients) noexcept;

        template<typename T>
        result<void> send_misc_data(T const& data, network_data_id_t data_id, client_id_t receipient_client_id = all_clients) noexcept;
        result<void> send_metadata(client_id_t recipient_client_id = all_clients) noexcept;

    private:
        template<typename T, typename A>
        result<void> relay_packet(T const& data, A const& aux_data, client_id_t channel, client_id_t receipient_client_id) noexcept;

    public:
        client_id_t initialize_client(client_endpoint& new_client, client_model model) noexcept;
        void reject_client(endpoint& uninitialized_client) noexcept;
        result<void> disconnect_client(client_id_t client_id, bool force = false) noexcept;
    public:
        constexpr void reset_client(client_id_t client_id) noexcept;

    public:
        result<void> process_network_data() noexcept;

        result<void> poll_network(timeout_ms_t timeout_ms = default_timeout_ms) noexcept;


    protected:
        callbacks_type callback_fns;

        std::array<client_endpoint*, max_num_clients> clients;
    };
}


namespace ggez::session {
    template<traits::network_transferrable PerStepDataT, auto ConfigV = session::config_t{}, traits::metadata_like MetadataT = session::metadata_t<ConfigV>, traits::network_transferrable StepT = std::size_t> requires traits::config_like<decltype(ConfigV)>
    using basic_host = basic<session::model::host, PerStepDataT, ConfigV, MetadataT, StepT>;
}

namespace ggez {
    template<traits::network_transferrable PerStepDataT, auto ConfigV = session::config_t{}, traits::metadata_like MetadataT = session::metadata_t<ConfigV>, traits::network_transferrable StepT = std::size_t> requires traits::config_like<decltype(ConfigV)>
    using basic_host_session = session::basic_host<PerStepDataT, ConfigV, MetadataT, StepT>;
}


#include "ggez/session/basic_host.inl"
