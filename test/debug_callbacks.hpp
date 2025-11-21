#pragma once
#include <cstring>
#include <span>
#include <format>
#include <syncstream>

#include <ggez/session/callbacks.hpp>

#include "./session_types.hpp"


namespace ggez::test::debug {
    using client_endpoint_type = typename host::client_endpoint;
    using client_functions_type = ggez::session::function<ggez::session::model::client, per_step_data_t, config, metadata_t, step_t>;
    
    using host_endpoint_type = typename client::host_endpoint;
    using host_functions_type = ggez::session::function<ggez::session::model::host, per_step_data_t, config, metadata_t, step_t>;
}


namespace ggez::test::debug {
    template<typename... S>
    void print_client_line(std::format_string<S...> f, S&&... s) noexcept {
        std::osyncstream(std::cout) << "[client] " << std::vformat(f.get(), std::make_format_args(s...)) << '\n';
    }

    template<typename... S>
    void print_host_line(std::format_string<S...> f, S&&... s) noexcept {
        std::osyncstream(std::cout) << "[host] " << std::vformat(f.get(), std::make_format_args(s...)) << '\n';
    }
}


namespace ggez::test::debug {
    constexpr static result<bool> 
    check_client_counts(host& session, client_endpoint_type& uninitialized_client, client_model requested_model) noexcept {
        print_host_line("New {} client connected. Checking if there's room for it...", to_string_view(requested_model));
        return host_functions_type::check_client_counts(session, uninitialized_client, requested_model);
    }

    constexpr static result<void> 
    broadcast_metadata(host& session, client_endpoint_type& client) noexcept {
        print_host_line("New {} client initialized. Broadcasting metadata...", to_string_view(client.info().model()));
        return host_functions_type::broadcast_metadata(session, client);
    }
    

    constexpr static result<void> 
    confirm_client_config(host& session, client_endpoint_type& client, std::span<const std::byte> data, network_data_id_t data_id) noexcept { 
        print_host_line("Confirming config of {} client (#{})...", to_string_view(client.info().model()), client.info().id());
        return host_functions_type::confirm_client_config(session, client, data, data_id);
    };

    constexpr static result<void> 
    broadcast_received_per_step_data(host& session, client_endpoint_type& sending_client, per_step_data_t data, step_t step) noexcept { 
        print_host_line("Broadcasting received per step data (step #{}) from {} client (#{})...", step, to_string_view(sending_client.info().model()), sending_client.info().id());
        return host_functions_type::broadcast_received_per_step_data(session, sending_client, data, step);
    };
}

namespace ggez::test::debug {
    constexpr static result<void> 
    send_config_for_confirmation(client& session, host_endpoint_type& host) noexcept {
        print_client_line("Sending config for confirmation...");
        return client_functions_type::send_config_for_confirmation(session, host);
    }


    constexpr static result<void> 
    verify_config_confirmation(client& session, host_endpoint_type& host, std::span<const std::byte> data, network_data_id_t data_id) noexcept {
        result<void> r = client_functions_type::verify_config_confirmation(session, host, data, data_id);
        if(r.has_value()) {
            print_client_line("Config successfully confirmed");
            return {};
        }
        switch(r.error()){
        case ggez::errc::session_config_mismatch:
            print_client_line("Config does not match the host's!");
            break;
        default:
            print_client_line("Unknown error when receiving config match confirmation");
            break;
        }
        return r;
    }

    constexpr static result<void> 
    copy_metadata(client& session, host_endpoint_type& host, metadata_t metadata) noexcept {
        print_client_line("Copying metadata from host...");
        return client_functions_type::copy_metadata(session, host, metadata);
    }

    constexpr static result<void> 
    push_back_remote_per_step_data(client& session, host_endpoint_type& host, per_step_data_t data, step_t step, client_id_t source_client_id, typename client::current_steps_type& current_steps, typename client::per_step_state_queue_span_type per_step_state_queue_span) noexcept {
        print_client_line("Copying per step data (step #{}) from host (originating from client #{})...", step, source_client_id);
        return client_functions_type::push_back_remote_per_step_data(session, host, data, step, source_client_id, current_steps, per_step_state_queue_span);
    }


    constexpr static result<void> 
    softly_drop_connection(client& session, host_endpoint_type& host, typename client::current_steps_type& current_steps, typename client::per_step_state_queue_span_type per_step_state_queue_span) noexcept {
        print_client_line("Softly dropping connection to host...");
        return client_functions_type::softly_drop_connection(session, host, current_steps, per_step_state_queue_span);
    }
}


namespace ggez::test::debug {
    constexpr typename host::callbacks_type host_callbacks{
        check_client_counts,
        broadcast_metadata,

        confirm_client_config,
        broadcast_received_per_step_data,

        nullptr
    };

    constexpr typename client::callbacks_type client_callbacks{
        send_config_for_confirmation,

        verify_config_confirmation,
        copy_metadata,
        push_back_remote_per_step_data,

        nullptr,
        softly_drop_connection,

        nullptr
    };
}