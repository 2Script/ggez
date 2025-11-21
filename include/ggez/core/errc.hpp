#pragma once
#include <cstdint>

#include <result.hpp>


namespace ggez::error {
    using code_t = std::int_fast16_t;
}

namespace ggez::error {
    enum code : code_t {
        none,
        
        connection_to_host_failed,
        connection_to_host_timed_out,
        session_initialization_failed,
        network_initialization_failed,
        network_data_send_failed,
        invalid_data_size,
        invalid_session_metadata,
        invalid_network_data_id,
        invalid_network_address,

        session_config_mismatch,
        wrong_client_id_for_data,

        num_codes,
    };
}

namespace ggez {
    using errc = error::code;
}

OL_RESULT_DECLARE_AS_ERROR_CODE(ggez::error, code, nullptr, nullptr, ggez)
