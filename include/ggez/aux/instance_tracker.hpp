#pragma once
#include <cstddef>
#include <atomic>

#include "ggez/core/result.hpp"


namespace ggez::impl {
    struct instance_tracker {
        inline instance_tracker() noexcept;
        inline ~instance_tracker() noexcept;

        inline result<void> new_instance() noexcept;
    };
}   

namespace ggez::impl {
    inline std::atomic<std::size_t>& instance_count() noexcept;
    
    inline bool& initialized() noexcept;
    inline std::mutex& initialized_mutex() noexcept;
}

#include "ggez/aux/instance_tracker.inl"