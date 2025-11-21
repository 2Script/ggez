#pragma once
#include "ggez/session/basic_client_base.hpp"
#include <atomic>
#include <memory>
#include <mutex>

#include <enet/enet.h>

#include "ggez/aux/from_bytes.hpp"
#include "ggez/infra/disconnect_reason.hpp"
#include "ggez/infra/discrepancy_policy.hpp"
#include "ggez/infra/ip_address.hpp"
#include "ggez/infra/client_model.hpp"
#include "ggez/infra/byte_order.hpp"
#include "ggez/core/client_id_t.hpp"
#include "ggez/infra/data_source.hpp"
#include "ggez/infra/network_address_view.hpp"
#include "ggez/infra/network_data_id.hpp"


namespace ggez::session::impl {
    template<
        template<session::model, traits::network_transferrable, auto, traits::metadata_like, traits::network_transferrable> typename DerivedTemplateT, 
        traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT
    > requires traits::config_like<decltype(ConfigV)>
    constexpr    basic_client_base<DerivedTemplateT, PerStepDataT, ConfigV, MetadataT, StepT>::
    basic_client_base(client_model model, MetadataT initial_metadata, callbacks_type session_callbacks) noexcept :
        base_type(initial_metadata, network_address_view{}, network_port{}), 
        callback_fns(session_callbacks), host_ptr{},
        //mismatch(std::make_unique<std::atomic<bool>>(true)), finished(std::make_unique<std::atomic<bool>>(false)), 
        //per_step_queue_size{queue_size + step_delay()}, per_step_queue(std::make_unique<per_step_state_type[]>(per_step_queue_size)),
        current_steps{}, //oldest_step_info{},
        my_client_id_mutex(std::make_unique<std::shared_mutex>()), metadata_mutex(std::make_unique<std::mutex>()), current_step_mutexes{}, //oldest_step_info_mutex(std::make_unique<std::mutex>()),
        player_count_cv(std::make_unique<std::condition_variable>()), valid_client_id_cv(std::make_unique<std::condition_variable_any>()),
        initialized_players{},
        //initialized_player_count(std::make_unique<std::atomic<std::size_t>>(1)), //player_count(std::make_unique<std::atomic<std::size_t>>(0)),
        my_client_model{model}, my_client_id{no_client_id}
    {
        for(std::size_t i = 0; i < max_num_players; ++i) {
            current_steps[i] = no_step;
            current_step_mutexes[i] = std::make_unique<std::mutex>();
        }

        //for(std::size_t i = 0; i < std::max(step_delay(), static_cast<std::size_t>(1)); ++i)
        //    for(std::size_t j = 0; j < max_num_players; ++j)    
        //        per_step_queue[i][j] = PerStepDataT{};
    }

    template<
        template<session::model, traits::network_transferrable, auto, traits::metadata_like, traits::network_transferrable> typename DerivedTemplateT, 
        traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT
    > requires traits::config_like<decltype(ConfigV)>
    result<void>    basic_client_base<DerivedTemplateT, PerStepDataT, ConfigV, MetadataT, StepT>::
    initialize() noexcept {
        RESULT_VERIFY(this->enet_instance_tracker.new_instance());

        this->handle = std::unique_ptr<ENetHost, ::ggez::impl::enet_host_deleter>(enet_host_create(nullptr,
            1, ConfigV.client_count_upper_limits[static_cast<std::size_t>(client_model::player)] + 1,
            0, 0
        ));
        if(!this->handle) return errc::session_initialization_failed;
        
        //if constexpr (ConfigV.step_mismatch_policy == discrepancy_policy::delay)

        return {};
    }
}


namespace ggez::session::impl {
    template<
        template<session::model, traits::network_transferrable, auto, traits::metadata_like, traits::network_transferrable> typename DerivedTemplateT, 
        traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT
    > requires traits::config_like<decltype(ConfigV)>
    template<std::convertible_to<PerStepDataT> PSD, std::convertible_to<StepT> S>
    void    basic_client_base<DerivedTemplateT, PerStepDataT, ConfigV, MetadataT, StepT>::
    push_back_remote(PSD&& remote_per_step_data, S&& step, client_id_t client_id) noexcept {
        return static_cast<derived_type>(this)->push_back_per_step_data<data_source::remote>(std::forward<PSD>(remote_per_step_data), std::forward<S>(step), client_id);
    }
    
    template<
        template<session::model, traits::network_transferrable, auto, traits::metadata_like, traits::network_transferrable> typename DerivedTemplateT, 
        traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT
    > requires traits::config_like<decltype(ConfigV)>
    template<std::convertible_to<PerStepDataT> PSD, std::convertible_to<StepT> S>
    void    basic_client_base<DerivedTemplateT, PerStepDataT, ConfigV, MetadataT, StepT>::
    push_back_local(PSD&& local_per_step_data, S&& step) noexcept {
        //std::shared_lock<std::shared_mutex> client_id_read_op_lk(*my_client_id_mutex);
        return static_cast<derived_type>(this)->push_back_per_step_data<data_source::local>(std::forward<PSD>(local_per_step_data), std::forward<S>(step), my_client_id);
    }
    
    template<
        template<session::model, traits::network_transferrable, auto, traits::metadata_like, traits::network_transferrable> typename DerivedTemplateT, 
        traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT
    > requires traits::config_like<decltype(ConfigV)>
    template<std::convertible_to<PerStepDataT> PSD, std::convertible_to<StepT> S>
    result<void>    basic_client_base<DerivedTemplateT, PerStepDataT, ConfigV, MetadataT, StepT>::
    send_local_per_step_data(PSD&& local_per_step_data, S&& step) noexcept {
        //std::shared_lock<std::shared_mutex> client_id_read_op_lk(*my_client_id_mutex);
        return send_packet<PerStepDataT, StepT>(std::forward<PSD>(local_per_step_data), std::forward<S>(step), my_client_id + 1);
    }
    

    //template<
    //    template<session::model, traits::network_transferrable, auto, traits::metadata_like, traits::network_transferrable> typename DerivedTemplateT, 
    //    traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT
    //> requires traits::config_like<decltype(ConfigV)>
    //template<std::convertible_to<PerStepDataT> PSD>
    //result<void>    basic_client_base<DerivedTemplateT, PerStepDataT, ConfigV, MetadataT, StepT>::
    //publish_push_back_local(PSD&& local_per_step_data) noexcept {
    //    const PerStepDataT data(std::forward<PSD>(local_per_step_data));
    //    RESULT_VERIFY(publish(data, network_data_id::per_step_data, my_client_id + 1));
    //    push_back_local(std::move(data));
    //    return {};
    //}


    template<
        template<session::model, traits::network_transferrable, auto, traits::metadata_like, traits::network_transferrable> typename DerivedTemplateT, 
        traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT
    > requires traits::config_like<decltype(ConfigV)>
    template<typename T>
    result<void>    basic_client_base<DerivedTemplateT, PerStepDataT, ConfigV, MetadataT, StepT>::
    send_misc_data(T&& data, network_data_id_t data_id) noexcept {
        return send_packet<std::remove_cvref_t<T>, network_data_id_t>(std::forward<T>(data), data_id, base_type::metadata_channel_id);
    }
}


namespace ggez::session::impl {
        template<
        template<session::model, traits::network_transferrable, auto, traits::metadata_like, traits::network_transferrable> typename DerivedTemplateT, 
        traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT
    > requires traits::config_like<decltype(ConfigV)>
    template<typename DataT, typename AuxDataT, typename T, typename A>
    result<void>    basic_client_base<DerivedTemplateT, PerStepDataT, ConfigV, MetadataT, StepT>::
    send_packet(T&& data, A&& aux_data, client_id_t channel) noexcept {
        const DataT network_data = ggez::byte_order::host_to_network(std::forward<T>(data));
        const AuxDataT aux_net_data = ggez::byte_order::host_to_network(std::forward<A>(aux_data));
        ENetPacket* packet = enet_packet_create(&network_data, sizeof(DataT), ConfigV.data_policy);
        enet_packet_resize(packet, sizeof(DataT) + sizeof(AuxDataT));
        std::memcpy(&packet->data[sizeof(DataT)], &aux_net_data, sizeof(AuxDataT));
        //constexpr static std::size_t offset = network_data_offset_v<T, A>;
        //enet_packet_resize(packet, offset + sizeof(A));
        //std::memcpy(&packet->data[offset], &aux_net_data, sizeof(A));

        if(enet_peer_send(host_ptr, channel, packet) < 0) 
            return errc::network_data_send_failed;
        return {};
    }
}



namespace ggez::session::impl {
    template<
        template<session::model, traits::network_transferrable, auto, traits::metadata_like, traits::network_transferrable> typename DerivedTemplateT, 
        traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT
    > requires traits::config_like<decltype(ConfigV)>
    bool    basic_client_base<DerivedTemplateT, PerStepDataT, ConfigV, MetadataT, StepT>::
    wait_until_acknowledged_by_host() const noexcept {
        //my_client_id->wait(no_client_id, std::memory_order::relaxed);
        std::shared_lock<std::shared_mutex> my_client_id_read_lk(*my_client_id_mutex);
        valid_client_id_cv->wait(my_client_id_read_lk);
        return false;
        //return !finished->load(std::memory_order::relaxed);
    }

    template<
        template<session::model, traits::network_transferrable, auto, traits::metadata_like, traits::network_transferrable> typename DerivedTemplateT, 
        traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT
    > requires traits::config_like<decltype(ConfigV)>
    bool    basic_client_base<DerivedTemplateT, PerStepDataT, ConfigV, MetadataT, StepT>::
    wait_until_enough_players_connected() const noexcept {
        std::unique_lock<std::mutex> metadata_lk(*metadata_mutex);
        player_count_cv->wait(metadata_lk);
        return false;
        //return !finished->load(std::memory_order::relaxed);
    }
}



namespace ggez::session::impl {
    template<
        template<session::model, traits::network_transferrable, auto, traits::metadata_like, traits::network_transferrable> typename DerivedTemplateT, 
        traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT
    > requires traits::config_like<decltype(ConfigV)>
    result<void>    basic_client_base<DerivedTemplateT, PerStepDataT, ConfigV, MetadataT, StepT>::
    connect(network_address_view host_addr, network_port host_port, enet_uint32 timeout_ms) noexcept {
        ENetAddress address{};
        if(enet_address_set_host(&address, host_addr.data()) < 0) 
            return errc::invalid_network_address;
        address.port = host_port;
        endpoint* peer = enet_host_connect(this->handle.get(), &address, max_num_players + 1, static_cast<enet_uint32>(my_client_model));
        if(!peer) return errc::connection_to_host_failed;

        ENetEvent event;
        if (enet_host_service(this->handle.get(), &event, timeout_ms) <= 0 || event.type != ENET_EVENT_TYPE_CONNECT)
            return errc::connection_to_host_timed_out;
        
        //Connected to host
        this->host_net_addr = host_addr;
        this->host_net_port = host_port;
        host_ptr = reinterpret_cast<host_endpoint*>(peer);
        std::destroy_at(&host_ptr->data);
        ::new (&host_ptr->data) host_info_type{};

        return this->invoke_callback(callback_fns.on_connect_to_host_fn, *this, *host_ptr);

    }

    template<
        template<session::model, traits::network_transferrable, auto, traits::metadata_like, traits::network_transferrable> typename DerivedTemplateT, 
        traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT
    > requires traits::config_like<decltype(ConfigV)>
    result<void>    basic_client_base<DerivedTemplateT, PerStepDataT, ConfigV, MetadataT, StepT>::
    connect_local(basic<session::model::host, PerStepDataT, ConfigV, MetadataT, StepT> const& host) noexcept {
        return connect(localhost, host.port());
    }


    template<
        template<session::model, traits::network_transferrable, auto, traits::metadata_like, traits::network_transferrable> typename DerivedTemplateT, 
        traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT
    > requires traits::config_like<decltype(ConfigV)>
    result<void>    basic_client_base<DerivedTemplateT, PerStepDataT, ConfigV, MetadataT, StepT>::
    disconnect(bool force) noexcept {
        if(!force) {
            host_ptr->info() = host_info_type{disconnect_reason::requested_locally};
            enet_peer_disconnect(host_ptr, disconnect_reason::requested_remotely);
            return {};
        }
    
        host_ptr->info() = host_info_type{disconnect_reason::forced};
        RESULT_VERIFY(this->invoke_callback(callback_fns.on_disconnect_from_host_fn, *this, *host_ptr));
        enet_peer_reset(host_ptr);
        reset();
        return {};
    }
}



namespace ggez::session::impl {
    template<
        template<session::model, traits::network_transferrable, auto, traits::metadata_like, traits::network_transferrable> typename DerivedTemplateT, 
        traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT
    > requires traits::config_like<decltype(ConfigV)>
    result<void>    basic_client_base<DerivedTemplateT, PerStepDataT, ConfigV, MetadataT, StepT>::
    process_network_data() noexcept {
        switch(this->current_event.type) {
        case ENET_EVENT_TYPE_RECEIVE:
            //Packet was sent from the host
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

                case network_data_id::client_id: {
                    if(this->current_event.packet->dataLength != sizeof(network_data_id_t) + sizeof(client_id_t)) [[unlikely]] {
                        enet_packet_destroy(this->current_event.packet);
                        return errc::invalid_data_size;
                    }

                    const client_id_t new_id = byte_order::network_to_host(from_bytes<client_id_t>(this->current_event.packet->data));
                    //my_client_id->store(new_id, std::memory_order::relaxed);
                    {
                    std::scoped_lock<std::shared_mutex, std::mutex> client_id_lock(*my_client_id_mutex, *current_step_mutexes[new_id]);
                    current_steps[new_id] = static_cast<StepT>(step_delay());
                    my_client_id = new_id;
                    }
                    initialized_players.set(new_id, true);
                    //initialized_player_count->fetch_add(1, std::memory_order::relaxed);
                    valid_client_id_cv->notify_all();
                    break;
                    }

                case network_data_id::metadata: {
                    if(this->current_event.packet->dataLength != sizeof(network_data_id_t) + sizeof(MetadataT)) [[unlikely]] {
                        enet_packet_destroy(this->current_event.packet);
                        return errc::invalid_data_size;
                    }

                    std::unique_lock<std::mutex> metadata_lk(*metadata_mutex);
                    RESULT_VERIFY(this->invoke_callback(callback_fns.on_recieve_metadata_from_host_fn, *this, *host_ptr, from_bytes<MetadataT>(this->current_event.packet->data)));

                    const std::bitset<max_num_players> uninitialized_players = this->metadata().active_client_ids.player_ids() ^ initialized_players;
                    for(std::size_t i = uninitialized_players._Find_first(); i < max_num_players; i = uninitialized_players._Find_next(i)) {
                        std::unique_lock<std::mutex> client_lock(*current_step_mutexes[i]);
                        initialized_players.set(i, true);
                        current_steps[i] = static_cast<StepT>(step_delay());
                    }
                    if(initialized_players.count() >= this->metadata().min_player_count)
                        player_count_cv->notify_all();
                        
                    break;
                    }
                default:
                    RESULT_VERIFY(this->invoke_callback(callback_fns.on_recieve_misc_data_from_host_fn, *this, *host_ptr, std::span{reinterpret_cast<std::byte const*>(this->current_event.packet->data), this->current_event.packet->dataLength - sizeof(network_data_id_t)}, data_id));
                    break;
                }
                break;
                }

            default: {
                const client_id_t data_client_id = this->current_event.channelID - 1;
                
                std::shared_lock<std::shared_mutex> my_client_id_read_lk(*my_client_id_mutex);
                if(data_client_id == my_client_id) break;
                my_client_id_read_lk.unlock();

                //constexpr static std::size_t step_offset = network_data_offset_v<PerStepDataT, StepT>;
                constexpr static std::size_t step_offset = sizeof(PerStepDataT);
                if(this->current_event.packet->dataLength != step_offset + sizeof(StepT)) [[unlikely]] {
                    enet_packet_destroy(this->current_event.packet);
                    return errc::invalid_data_size;
                }
                //StepT const* step_ptr = start_lifetime_as<StepT>(&this->current_event.packet->data[step_offset]);
                
                RESULT_VERIFY(this->invoke_callback(callback_fns.on_recieve_per_step_data_from_host_fn, *this, *host_ptr, from_bytes<PerStepDataT>(this->current_event.packet->data), from_bytes<StepT>(&this->current_event.packet->data[step_offset]), data_client_id, current_steps, per_step_state_queue_span_type{per_step_queue.get(), per_step_queue_size}))
                break;
                }
            }
            enet_packet_destroy(this->current_event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            //Disconnected from host
            RESULT_VERIFY(this->invoke_callback(callback_fns.on_disconnect_from_host_fn, *this, *host_ptr));
            reset();
            break;
        default:
            break;
        }

        return{};
    }


    template<
        template<session::model, traits::network_transferrable, auto, traits::metadata_like, traits::network_transferrable> typename DerivedTemplateT, 
        traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT
    > requires traits::config_like<decltype(ConfigV)>
    result<void>    basic_client_base<DerivedTemplateT, PerStepDataT, ConfigV, MetadataT, StepT>::
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



namespace ggez::session::impl {
    template<
        template<session::model, traits::network_transferrable, auto, traits::metadata_like, traits::network_transferrable> typename DerivedTemplateT, 
        traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT
    > requires traits::config_like<decltype(ConfigV)>
    void    basic_client_base<DerivedTemplateT, PerStepDataT, ConfigV, MetadataT, StepT>::
    reset() noexcept {
        this->host_net_addr = network_address_view{};
        this->host_net_port = 0;


        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            std::scoped_lock write_op_lk(*my_client_id_mutex, *metadata_mutex, (*current_step_mutexes[Is])...);
            for(std::size_t i = 0; i < max_num_players; ++i) 
                current_steps[i] = no_step;
            //finished->store(true, std::memory_order::relaxed);
            player_count_cv->notify_all();
            valid_client_id_cv->notify_all();
            //my_client_id->store(no_client_id, std::memory_order::relaxed);
            //my_client_id->notify_all();
            std::destroy_at(&host_ptr->info());
            host_ptr = nullptr;
            initialized_players = {};
            this->metadata() = {};
        }(std::make_index_sequence<max_num_players>{});
        //finished->store(false, std::memory_order::relaxed);
    }
}