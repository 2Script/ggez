#pragma once
#include <result.hpp>

#include "errc.hpp"


namespace ggez {
    template<typename T>
    using result = ol::result<T, errc>;
}
