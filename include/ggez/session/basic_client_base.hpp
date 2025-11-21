#pragma once
#include <concepts>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <shared_mutex>
#include <type_traits>
#include <mutex>

#include "ggez/infra/host_info.hpp"
#include "ggez/infra/network_data_id.hpp"
#include "ggez/session/callbacks.hpp"
#include "ggez/core/client_id_t.hpp"
#include "ggez/infra/client_model.hpp"
#include "ggez/infra/data_source.hpp"
#include "ggez/infra/disconnect_reason.hpp"
#include "ggez/infra/discrepancy_policy.hpp"
#include "ggez/infra/endpoint.hpp"
#include "ggez/infra/network_address_view.hpp"
#include "ggez/infra/network_port.hpp"
#include "ggez/infra/ip_address.hpp"
#include "ggez/infra/network_stats_t.hpp"
#include "ggez/session/metadata_t.hpp"
#include "ggez/session/config_t.hpp"
#include "ggez/infra/step_info.hpp"
#include "ggez/session/basic.hpp"
#include "ggez/session/basic_base.hpp"
#include "ggez/session/model.hpp"
#include "ggez/traits/is_implicit_lifetime.hpp"


namespace ggez::session::impl {
    template<
        template<session::model, traits::network_transferrable, auto, traits::metadata_like, traits::network_transferrable> typename DerivedTemplateT, 
        traits::network_transferrable PerStepDataT, auto ConfigV, traits::metadata_like MetadataT, traits::network_transferrable StepT
    > requires traits::config_like<decltype(ConfigV)>
    class basic_client_base : public impl::basic_base<PerStepDataT, ConfigV, MetadataT, StepT> {
    public:
        using callbacks_type = session::callbacks<session::model::client, PerStepDataT, ConfigV, MetadataT, StepT>;
        using base_type = impl::basic_base<PerStepDataT, ConfigV, MetadataT, StepT>;
        using derived_type = DerivedTemplateT<session::model::client, PerStepDataT, ConfigV, MetadataT, StepT>;

    public:
        constexpr static std::size_t max_num_players = ConfigV.client_count_upper_limits[static_cast<std::size_t>(client_model::player)];

        //TODO: maybe use unique_ptr<[]> instead of vector?
        using per_step_state_type = std::array<PerStepDataT, max_num_players>;
        using per_step_state_queue_span_type = std::span<per_step_state_type>;
        using current_steps_type = std::array<StepT, max_num_players>;

        using host_info_type = host_info;
        using host_endpoint = session_endpoint<host_info_type>;


    public:
        constexpr basic_client_base() noexcept = default;
        constexpr basic_client_base(client_model client_model, MetadataT initial_metadata = {}, callbacks_type session_callbacks = {}) noexcept;

        result<void> initialize() noexcept;

    public:
        constexpr network_address_view const& host_address() const noexcept { return this->host_net_addr; }
        constexpr network_port         const& host_port()    const noexcept { return this->host_net_port; }

    public:
        std::array<PerStepDataT, max_num_players> const& front() const noexcept;
    public:
        template<std::convertible_to<PerStepDataT> PSD, std::convertible_to<StepT> S>
        void push_back_remote(PSD&& remote_per_step_data, S&& step, client_id_t client_id) noexcept;
        template<std::convertible_to<PerStepDataT> PSD, std::convertible_to<StepT> S>
        void push_back_local(PSD&& local_per_step_data, S&& step) noexcept;
        template<std::convertible_to<PerStepDataT> PSD, std::convertible_to<StepT> S>
        result<void> send_local_per_step_data(PSD&& local_per_step_data, S&& step) noexcept;

        //template<std::convertible_to<PerStepDataT> PSD>
        //result<void> publish_push_back_local(PSD&& local_per_step_data) noexcept;


        template<typename T>
        result<void> send_misc_data(T&& data, network_data_id_t data_id) noexcept;

    protected:
        template<data_source DataSource, std::convertible_to<PerStepDataT> PSD, std::convertible_to<StepT> S>
        void push_back_per_step_data(PSD&& per_step_data, S&& step, client_id_t client_id) noexcept requires (ConfigV.step_mismatch_policy != discrepancy_policy::ignore);
        template<data_source DataSource, std::convertible_to<PerStepDataT> PSD, std::convertible_to<StepT> S>
        void push_back_per_step_data(PSD&& per_step_data, S&& step, client_id_t client_id) noexcept requires (ConfigV.step_mismatch_policy == discrepancy_policy::ignore);
        
        template<typename DataT, typename AuxDataT, typename T, typename A>
        result<void> send_packet(T&& data, A&& aux_data, client_id_t channel) noexcept;

        constexpr result<void> check_for_mismatch(std::size_t step_diff) noexcept;
        
        constexpr std::size_t step_delay() const noexcept;
        

    public:
        bool wait_until_acknowledged_by_host() const noexcept;
        bool wait_until_enough_players_connected() const noexcept;
        bool wait_until_no_mismatch() const noexcept;

    
    public:
        result<void> connect(network_address_view host_addr, network_port host_port, enet_uint32 timeout_ms = 5000) noexcept;
        result<void> connect_local(basic<session::model::host, PerStepDataT, ConfigV, MetadataT, StepT> const& host) noexcept;
        result<void> disconnect(bool force = false) noexcept;

    public:
        result<void> process_network_data() noexcept;

        result<void> poll_network(timeout_ms_t timeout_ms = default_timeout_ms) noexcept;


    public:
        void reset() noexcept;

    protected:
        constexpr static std::size_t queue_size = [](){ if constexpr(ConfigV.step_mismatch_policy == discrepancy_policy::rollback) return ConfigV.max_prediction_steps; else return 1;}();

        constexpr static StepT no_step = static_cast<StepT>(static_cast<std::size_t>(-1));
    protected:
        callbacks_type callback_fns;
        host_endpoint* host_ptr;

        //std::unique_ptr<std::atomic<bool>> mismatch;
        //std::unique_ptr<std::atomic<bool>> finished;

        //std::size_t per_step_queue_size;
        //std::unique_ptr<per_step_state_type[]> per_step_queue;

        current_steps_type current_steps;
        //step_info<StepT, max_num_players> oldest_step_info;

        std::unique_ptr<std::shared_mutex> my_client_id_mutex;
        std::unique_ptr<std::mutex> metadata_mutex;
        std::array<std::unique_ptr<std::mutex>, max_num_players> current_step_mutexes;
        //std::unique_ptr<std::mutex> oldest_step_info_mutex;

        std::unique_ptr<std::condition_variable> player_count_cv;
        std::unique_ptr<std::condition_variable_any> valid_client_id_cv;

        std::bitset<max_num_players> initialized_players;

        client_model my_client_model;
        //std::unique_ptr<std::atomic<client_id_t>> my_client_id;
        client_id_t my_client_id;

    };
}

#include "ggez/session/basic_client.inl"
