#pragma once
#include "ggez/session/basic_host.hpp"
#include <cstring>
#include <memory>

#include <enet/enet.h>

#include "ggez/infra/client_model.hpp"
#include "ggez/infra/ip_address.hpp"
#include "ggez/infra/byte_order.hpp"
#include "ggez/core/client_id_t.hpp"
#include "ggez/infra/disconnect_reason.hpp"
#include "ggez/infra/network_data_id.hpp"


namespace ggez::session {
    template<traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT> requires traits::config_like<decltype(ConfigV)>
    constexpr    basic<session::model::host, PerStepDataT, ConfigV, MetadataT, StepT>::
    basic(network_address_view addr, network_port port, MetadataT initial_metadata, callbacks_type session_callbacks) noexcept :
        base_type(initial_metadata, addr, port), 
        callback_fns(session_callbacks), clients{} {}


    template<traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT> requires traits::config_like<decltype(ConfigV)>
    result<void>    basic<session::model::host, PerStepDataT, ConfigV, MetadataT, StepT>::
    initialize() noexcept {
        RESULT_VERIFY(this->enet_instance_tracker.new_instance());

        ENetAddress address;
        if(this->host_net_addr.size() == 0 || (this->host_net_addr.size() >= any_host.size() && std::memcmp(this->host_net_addr.data(), any_host.data(), any_host.size()) == 0))
            address.host = ENET_HOST_ANY;
        else if(enet_address_set_host(&address, this->host_net_addr.data()) < 0)
            return errc::invalid_network_address;
        address.port = this->host_net_port;

        this->handle = std::unique_ptr<ENetHost, ::ggez::impl::enet_host_deleter>(enet_host_create(&address,
            ConfigV.client_count_upper_limits[static_cast<std::size_t>(client_model::player)] + ConfigV.client_count_upper_limits[static_cast<std::size_t>(client_model::spectator)],
            ConfigV.client_count_upper_limits[static_cast<std::size_t>(client_model::player)] + 1,
            0, 0
        ));
        if(!this->handle) return errc::session_initialization_failed;
        return {};
    }
}


namespace ggez::session {
    template<traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT> requires traits::config_like<decltype(ConfigV)>
    result<void>    basic<session::model::host, PerStepDataT, ConfigV, MetadataT, StepT>::
    relay_per_step_data(PerStepDataT data, StepT step, client_id_t data_source_client_id, client_id_t recipient_client_id) noexcept {
        return relay_packet<PerStepDataT>(data, step, data_source_client_id + 1, recipient_client_id);
    }

    template<traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT> requires traits::config_like<decltype(ConfigV)>
    result<void>    basic<session::model::host, PerStepDataT, ConfigV, MetadataT, StepT>::
    send_per_step_data(PerStepDataT data, StepT step, client_id_t data_source_client_id, client_id_t recipient_client_id) noexcept {
        const PerStepDataT network_data = byte_order::host_to_network(data);
        return relay_per_step_data(network_data, step, data_source_client_id, recipient_client_id);
    }


    template<traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT> requires traits::config_like<decltype(ConfigV)>
    template<typename T>
    result<void>    basic<session::model::host, PerStepDataT, ConfigV, MetadataT, StepT>::
    send_misc_data(T const& data, network_data_id_t data_id, client_id_t recipient_client_id) noexcept {
        const T network_data = byte_order::host_to_network(data);
        return relay_packet<T>(network_data, data_id, base_type::metadata_channel_id, recipient_client_id);
    }

    template<traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT> requires traits::config_like<decltype(ConfigV)>
    result<void>    basic<session::model::host, PerStepDataT, ConfigV, MetadataT, StepT>::
    send_metadata(client_id_t recipient_client_id) noexcept {
        return send_misc_data<MetadataT>(this->session_metadata, network_data_id::metadata, recipient_client_id);
    }
}

namespace ggez::session {
    template<traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT> requires traits::config_like<decltype(ConfigV)>
    template<typename T, typename A>
    result<void>    basic<session::model::host, PerStepDataT, ConfigV, MetadataT, StepT>::
    relay_packet(T const& data, A const& aux_data, client_id_t channel, client_id_t recipient_client_id) noexcept {
        ENetPacket* packet = enet_packet_create(&data, sizeof(T), network_data_policy_flag::reliable);
        enet_packet_resize(packet, sizeof(T) + sizeof(A));
        std::memcpy(&packet->data[sizeof(T)], &aux_data, sizeof(A));


        switch(recipient_client_id) {
        case all_clients:
            enet_host_broadcast(this->handle.get(), channel, packet);
            break;
        default:
            if(enet_peer_send(clients[recipient_client_id], channel, packet) < 0) 
                return errc::network_data_send_failed;
            break;
        }
        return {};
    }
}




namespace ggez::session {
    template<traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT> requires traits::config_like<decltype(ConfigV)>
    client_id_t    basic<session::model::host, PerStepDataT, ConfigV, MetadataT, StepT>::
    initialize_client(client_endpoint& new_client, client_model model) noexcept {
        const client_id_t new_client_id = this->session_metadata.active_client_ids.find_first_free_client_id(model);
        this->session_metadata.active_client_ids.set(new_client_id, true);
        std::destroy_at(&new_client.data);
        ::new (&new_client.data) client_info_type{new_client_id};
        clients[new_client_id] = &new_client;
        return new_client_id;
    }

    template<traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT> requires traits::config_like<decltype(ConfigV)>
    void    basic<session::model::host, PerStepDataT, ConfigV, MetadataT, StepT>::
    reject_client(endpoint& uninitialized_client) noexcept {
        return enet_peer_disconnect(&uninitialized_client, disconnect_reason::requested_remotely);
    }

    template<traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT> requires traits::config_like<decltype(ConfigV)>
    result<void>    basic<session::model::host, PerStepDataT, ConfigV, MetadataT, StepT>::
    disconnect_client(client_id_t client_id, bool force) noexcept {
        if(!force) {
            clients[client_id]->info() = client_info_type{clients[client_id]->info().id(), disconnect_reason::requested_locally};
            enet_peer_disconnect(&clients[client_id], disconnect_reason::requested_remotely);
            return {};
        }
    
        clients[client_id]->info() = client_info_type{clients[client_id]->info().id(), disconnect_reason::forced};
        RESULT_VERIFY(this->invoke_callback(callback_fns.on_client_disconnect_fn, *this, clients[client_id]));
        enet_peer_reset(&clients[client_id]);
        reset_client(clients[client_id]);
        return {};
    }


    template<traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT> requires traits::config_like<decltype(ConfigV)>
    constexpr void    basic<session::model::host, PerStepDataT, ConfigV, MetadataT, StepT>::
    reset_client(client_id_t client_id) noexcept {
        std::destroy_at(&clients[client_id]->info());
        this->session_metadata.active_client_ids.reset(client_id);
        clients[client_id] = nullptr;
    }
}



namespace ggez::session {
    template<traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT> requires traits::config_like<decltype(ConfigV)>
    result<void>    basic<session::model::host, PerStepDataT, ConfigV, MetadataT, StepT>::
    process_network_data() noexcept {
        switch(this->current_event.type) {
        case ENET_EVENT_TYPE_CONNECT: {
            //New client has connected
            client_endpoint* new_client = reinterpret_cast<client_endpoint*>(this->current_event.peer);
            const client_model new_client_model = static_cast<client_model>(this->current_event.data);
            RESULT_VERIFY_UNSCOPED(this->invoke_callback(callback_fns.on_client_connect_fn, *this, *new_client, new_client_model), should_init);
            if(!*should_init) break;
            
            const client_id_t new_client_id = initialize_client(*new_client, new_client_model);
            RESULT_VERIFY(send_misc_data(new_client_id, network_data_id::client_id, new_client_id));
            RESULT_VERIFY(this->invoke_callback(callback_fns.on_client_initialize_fn, *this, *new_client));
            break;
            }
            
        case ENET_EVENT_TYPE_RECEIVE: {
            //Packet was sent from a client
            client_endpoint* client = reinterpret_cast<client_endpoint*>(this->current_event.peer);

            switch(this->current_event.channelID) {
            case base_type::metadata_channel_id: {
                if(this->current_event.packet->dataLength <= sizeof(network_data_id_t)) [[unlikely]] {
                    enet_packet_destroy(this->current_event.packet);
                    return errc::invalid_data_size;
                }

                const network_data_id_t data_id = from_bytes<network_data_id_t>(&this->current_event.packet->data[this->current_event.packet->dataLength - sizeof(network_data_id_t)]);
                switch(data_id) {
                [[unlikely]] case network_data_id::per_step_data: 
                    enet_packet_destroy(this->current_event.packet);
                    return errc::invalid_network_data_id;
                default:
                    RESULT_VERIFY(this->invoke_callback(callback_fns.on_recieve_client_misc_data_fn, *this, *client, std::span{reinterpret_cast<std::byte const*>(this->current_event.packet->data), this->current_event.packet->dataLength - sizeof(network_data_id_t)}, data_id));
                    break;
                }

                break;
                }
            default: {
                //constexpr static std::size_t step_offset = network_data_offset_v<PerStepDataT, StepT>;
                constexpr static std::size_t step_offset = sizeof(PerStepDataT);
                if(this->current_event.packet->dataLength != (step_offset + sizeof(StepT))) [[unlikely]] {
                    enet_packet_destroy(this->current_event.packet);
                    return errc::invalid_data_size;
                }
                if(this->current_event.channelID - 1 != client->info().id()) [[unlikely]] {
                    enet_packet_destroy(this->current_event.packet);
                    return errc::wrong_client_id_for_data;
                }
                //StepT const* step_ptr = start_lifetime_as<StepT>(&this->current_event.packet->data[step_offset]);

                RESULT_VERIFY(this->invoke_callback(callback_fns.on_recieve_client_per_step_data_fn, *this, *client, from_bytes<PerStepDataT>(this->current_event.packet->data), from_bytes<StepT>(&this->current_event.packet->data[step_offset])))
                break;
                }
            }
            enet_packet_destroy(this->current_event.packet);
            break;
            }

        case ENET_EVENT_TYPE_DISCONNECT: {
            //Client has disconnected
            client_endpoint* old_client = reinterpret_cast<client_endpoint*>(this->current_event.peer);
            
            RESULT_VERIFY(this->invoke_callback(callback_fns.on_client_disconnect_fn, *this, *old_client));
            reset_client(old_client->info().id());
            break;
            }
        default:
            break;
        }

        return {};
    }

    template<traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT> requires traits::config_like<decltype(ConfigV)>
    result<void>    basic<session::model::host, PerStepDataT, ConfigV, MetadataT, StepT>::
    poll_network(timeout_ms_t timeout_ms) noexcept {
        int poll_status = 0;
        switch(timeout_ms) {
        case no_timeout:
            while((poll_status = this->active(default_timeout_ms)) >= 0)
                RESULT_VERIFY(process_network_data());
            break;
        default:
            while((poll_status = this->active(timeout_ms)) > 0)
                RESULT_VERIFY(process_network_data());
            break;
        }
        if(poll_status == 0) return {};
        return static_cast<errc>(poll_status);
    }
}
