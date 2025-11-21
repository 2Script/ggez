#pragma once
#include <span>

#include "ggez/infra/client_model.hpp"
#include "ggez/infra/network_data_id.hpp"
#include "ggez/session/config_t.hpp"
#include "ggez/session/function.hpp"
#include "ggez/infra/disconnect_reason.hpp"
#include "ggez/session/model.hpp"
#include "ggez/core/result.hpp"
#include "ggez/session/basic.hpp"


namespace ggez::session {
    template<session::model SessionModel, traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT> requires traits::config_like<decltype(ConfigV)>
    struct callbacks;
}


namespace ggez::session {
    template<traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT> requires traits::config_like<decltype(ConfigV)>
    struct callbacks<session::model::host, PerStepDataT, ConfigV, MetadataT, StepT> {
        using session_type = session::basic<session::model::host, PerStepDataT, ConfigV, MetadataT, StepT>;
        using session_endpoint_type = typename session_type::client_endpoint;

    public:
        using on_client_connect_fn_type           = result<bool>(*)(session_type& session, session_endpoint_type& uninitialized_client, client_model requested_model);
        using on_client_initialize_fn_type        = result<void>(*)(session_type& session, session_endpoint_type& client);

        using on_recieve_client_misc_data_fn_type     = result<void>(*)(session_type& session, session_endpoint_type& client, std::span<const std::byte> data, network_data_id_t data_id);
        using on_recieve_client_per_step_data_fn_type = result<void>(*)(session_type& session, session_endpoint_type& client, PerStepDataT data, StepT step);

        using on_client_disconnect_fn_type = result<void>(*)(session_type& session, session_endpoint_type& client);  

    public:
        on_client_connect_fn_type    on_client_connect_fn           = session::function<session::model::host, PerStepDataT, ConfigV, MetadataT, StepT>::check_client_counts;
        on_client_initialize_fn_type on_client_initialize_fn        = session::function<session::model::host, PerStepDataT, ConfigV, MetadataT, StepT>::broadcast_metadata;

        on_recieve_client_misc_data_fn_type     on_recieve_client_misc_data_fn     = session::function<session::model::host, PerStepDataT, ConfigV, MetadataT, StepT>::confirm_client_config;
        on_recieve_client_per_step_data_fn_type on_recieve_client_per_step_data_fn = session::function<session::model::host, PerStepDataT, ConfigV, MetadataT, StepT>::broadcast_received_per_step_data;

        on_client_disconnect_fn_type              on_client_disconnect_fn              = nullptr; //TODO: make this function after fully understanding how enet disonnect event works

    };
}

namespace ggez::session {
    template<traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT> requires traits::config_like<decltype(ConfigV)>
    struct callbacks<session::model::client, PerStepDataT, ConfigV, MetadataT, StepT> {
        using session_type = basic<session::model::client, PerStepDataT, ConfigV, MetadataT, StepT>;
        using session_endpoint_type = typename session_type::host_endpoint;
    public:
        using current_steps_type = typename session_type::current_steps_type;
        using per_step_state_queue_span_type = typename session_type::per_step_state_queue_span_type;
    
    public:
        using on_connect_to_host_fn_type = result<void>(*)(session_type& session, session_endpoint_type& host);

        using on_recieve_misc_data_from_host_fn_type     = result<void>(*)(session_type& session, session_endpoint_type& host, std::span<const std::byte> data, network_data_id_t data_id);
        using on_recieve_metadata_from_host_fn_type      = result<void>(*)(session_type& session, session_endpoint_type& host, MetadataT metadata);
        using on_recieve_per_step_data_from_host_fn_type = result<void>(*)(session_type& session, session_endpoint_type& host, PerStepDataT data, StepT step, client_id_t source_client_id, current_steps_type& current_steps, per_step_state_queue_span_type per_step_queue);

        using on_mismatch_fn_type = result<void>(*)(session_type& session, session_endpoint_type& host, current_steps_type& current_steps, per_step_state_queue_span_type per_step_queue);
        using on_max_prediction_steps_reached_fn_type = result<void>(*)(session_type& session, session_endpoint_type& host, current_steps_type& current_steps, per_step_state_queue_span_type per_step_queue);

        using on_disconnect_from_host_fn_type = result<void>(*)(session_type& session, session_endpoint_type& host);  

    public:
        on_connect_to_host_fn_type on_connect_to_host_fn = session::function<session::model::client, PerStepDataT, ConfigV, MetadataT, StepT>::send_config_for_confirmation;

        on_recieve_misc_data_from_host_fn_type     on_recieve_misc_data_from_host_fn     = session::function<session::model::client, PerStepDataT, ConfigV, MetadataT, StepT>::verify_config_confirmation;
        on_recieve_metadata_from_host_fn_type      on_recieve_metadata_from_host_fn      = session::function<session::model::client, PerStepDataT, ConfigV, MetadataT, StepT>::copy_metadata;
        on_recieve_per_step_data_from_host_fn_type on_recieve_per_step_data_from_host_fn = session::function<session::model::client, PerStepDataT, ConfigV, MetadataT, StepT>::push_back_remote_per_step_data;

        on_mismatch_fn_type                     on_mismatch_fn                     = nullptr; //TODO: rollback function that gets called by this function
        on_max_prediction_steps_reached_fn_type on_max_prediction_steps_reached_fn = session::function<session::model::client, PerStepDataT, ConfigV, MetadataT, StepT>::softly_drop_connection;

        on_disconnect_from_host_fn_type on_disconnect_from_host_fn = nullptr;
    };
}