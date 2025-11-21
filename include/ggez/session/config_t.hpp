#pragma once
#include <cstdint>
#include <type_traits>


#include "ggez/aux/byte_swap.hpp"
#include "ggez/infra/client_counts_t.hpp"
#include "ggez/infra/network_data_policy.hpp"
#include "ggez/infra/discrepancy_policy.hpp"


namespace ggez::session {
    struct config_t {
        discrepancy_policy step_mismatch_policy;
        network_data_policy data_policy = network_data_policy_flag::reliable;
        
        client_counts_t client_count_upper_limits{64, 64};

        

        //std::uint_least16_t max_prediction_steps = 8;
    };
}


namespace ggez::session {
    constexpr config_t byte_swap(config_t val) noexcept {
        using ::ggez::byte_swap;

        return {
            byte_swap(val.step_mismatch_policy),
            byte_swap(val.data_policy),
            byte_swap(val.client_count_upper_limits)
        };
    }
}
