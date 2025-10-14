#pragma once
#include <cstdint>

#include <result.hpp>


namespace ggez::error {
    using code_t = std::int_fast16_t;
}

namespace ggez::error {
    enum code : code_t {
        none,

        invalid_session,      
        invalid_player_handle,
        player_index_out_of_bounds,  
        prediction_threshold_reached, 
        operation_unsupported,          
        not_synchronized,     
        in_rollback,          
        input_dropped,        
        player_disconnected,  
        too_many_spectators,  
        invalid_request,      

        num_codes,
    };
}

namespace ggez {
    using errc = error::code;
}

OL_RESULT_DECLARE_AS_ERROR_CODE(ggez::error, code, nullptr, nullptr, ggez)
