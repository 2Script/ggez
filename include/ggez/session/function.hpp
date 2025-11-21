#pragma once
#include <cstring>
#include <span>

#include "ggez/aux/from_bytes.hpp"
#include "ggez/infra/byte_order.hpp"
#include "ggez/infra/client_model.hpp"
#include "ggez/infra/network_data_id.hpp"
#include "ggez/session/model.hpp"
#include "ggez/core/result.hpp"
#include "ggez/session/basic.hpp"


namespace ggez::session {
    template<session::model SessionModel, traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT> requires traits::config_like<decltype(ConfigV)>
    struct function;
}

namespace ggez::session {
    template<traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT> requires traits::config_like<decltype(ConfigV)>
    struct function<session::model::host, PerStepDataT, ConfigV, MetadataT, StepT> {
        using session_type = session::basic<session::model::host, PerStepDataT, ConfigV, MetadataT, StepT>;
        using session_endpoint_type = typename session_type::client_endpoint;

    public:
        //Default callback for "on_too_many_cilents_connect_fn" event
        constexpr static result<bool> 
        check_client_counts(session_type& session, session_endpoint_type&, client_model requested_model) noexcept {
            const client_counts_t client_counts = session.metadata().active_client_ids.count_clients();
            const std::size_t new_client_model_idx = static_cast<std::size_t>(requested_model);
            if(client_counts[new_client_model_idx] >= session.metadata().max_client_counts[new_client_model_idx]) return false;
            return true;
        }

        //Default callback for "on_client_connect_fn" event
        constexpr static result<void> 
        broadcast_metadata(session_type& session, session_endpoint_type&) noexcept {
            return session.send_metadata();
        }

    public:
        //Default callback for "on_recieve_client_misc_data" event
        constexpr static result<void> 
        confirm_client_config(session_type& session, session_endpoint_type& client, std::span<const std::byte> data, network_data_id_t data_id) noexcept { 
            if(data_id != network_data_id::config) return {};
            if(data.size_bytes() != sizeof(decltype(ConfigV)))
                return session.send_misc_data(false, network_data_id::confirmation, client.info().id());
        
            const decltype(ConfigV) config_data = byte_order::network_to_host(from_bytes<decltype(ConfigV)>(data.data()));
            constexpr decltype(ConfigV) local_config = ConfigV;
            const bool configs_match = std::memcmp(&local_config, &config_data, sizeof(decltype(ConfigV))) == 0;
            return session.send_misc_data(configs_match, network_data_id::confirmation, client.info().id());
        };

        //Default callback for "on_recieve_client_per_step_data" event
        constexpr static result<void> 
        broadcast_received_per_step_data(session_type& session, session_endpoint_type& sending_client, PerStepDataT data, StepT step) noexcept { 
            if(sending_client.info().model() == client_model::spectator) [[unlikely]] return {};
            return session.relay_per_step_data(data, step, sending_client.info().id());
        };
    };
}

namespace ggez::session {
    template<traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT> requires traits::config_like<decltype(ConfigV)>
    struct function<session::model::client, PerStepDataT, ConfigV, MetadataT, StepT> {
        using session_type = session::basic<session::model::client, PerStepDataT, ConfigV, MetadataT, StepT>;
        using session_endpoint_type = typename session_type::host_endpoint;

    public:
        //Default callback for "on_connnect_to_host" event
        constexpr static result<void> 
        send_config_for_confirmation(session_type& session, session_endpoint_type&) noexcept {
            return session.send_misc_data(ConfigV, network_data_id::config);
        }

    public:
        //Default callback for "on_recieve_misc_data_from_host" event
        constexpr static result<void> 
        verify_config_confirmation(session_type&, session_endpoint_type&, std::span<const std::byte> data, network_data_id_t data_id) noexcept {
            if(data_id != network_data_id::confirmation) return {};
            if(data.size_bytes() != sizeof(bool)) return errc::invalid_data_size;
        
            const bool config_matches = byte_order::network_to_host(from_bytes<bool>(data.data()));
            if(!config_matches) return errc::session_config_mismatch;
            return {};
        }

        //Default callback for "on_recieve_metadata_from_host" event
        constexpr static result<void> 
        copy_metadata(session_type& session, session_endpoint_type&, MetadataT metadata) noexcept {
            session.metadata() = byte_order::network_to_host(metadata);
            return {};
        }

        //Default callback for "on_recieve_per_step_data_from_host" event
        constexpr static result<void> 
        push_back_remote_per_step_data(session_type& session, session_endpoint_type&, PerStepDataT data, StepT step, client_id_t source_client_id, typename session_type::current_steps_type&, typename session_type::per_step_state_queue_span_type) noexcept {
            session.push_back_remote(byte_order::network_to_host(data), byte_order::network_to_host(step), source_client_id);
            return {};
        }

    public:
        //Default callback for "on_max_prediction_steps_reached_fn" event
        constexpr static result<void> 
        softly_drop_connection(session_type& session, session_endpoint_type&, typename session_type::current_steps_type&, typename session_type::per_step_state_queue_span_type) noexcept {
            return session.disconnect();
        }
    };
}
