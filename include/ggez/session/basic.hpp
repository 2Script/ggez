#pragma once 
#include <cstddef>

#include "ggez/session/config_t.hpp"
#include "ggez/session/metadata_t.hpp"
#include "ggez/session/model.hpp"
#include "ggez/traits/is_network_transferrable.hpp"
#include "ggez/traits/metadata_like.hpp"
#include "ggez/traits/config_like.hpp"


namespace ggez::session {
    template<session::model SessionType, traits::network_transferrable PerStepDataT, auto ConfigV = session::config_t{}, traits::metadata_like MetadataT = session::metadata_t<ConfigV>, traits::network_transferrable StepT = std::size_t> requires traits::config_like<decltype(ConfigV)>
    class basic;
}

namespace ggez {
    template<session::model SessionType, traits::network_transferrable PerStepDataT, auto ConfigV = session::config_t{}, traits::metadata_like MetadataT = session::metadata_t<ConfigV>, traits::network_transferrable StepT = std::size_t> requires traits::config_like<decltype(ConfigV)>
    using basic = session::basic<SessionType, PerStepDataT, ConfigV, MetadataT, StepT>;
}