#include "ggez/aux/instance_tracker.hpp"

#include <atomic>
#include <mutex>

#include <enet/enet.h>


namespace ggez::impl {
    result<void> instance_tracker::new_instance() noexcept {
        std::unique_lock<std::mutex> lk(initialized_mutex());
        if(initialized()) return {};
        if(enet_initialize() < 0) return errc::network_initialization_failed;
        initialized() = true;
        return {};
    }
    
    instance_tracker::instance_tracker() noexcept {
        instance_count().fetch_add(1, std::memory_order::relaxed);
    }

    instance_tracker::~instance_tracker() noexcept {
        if(instance_count().fetch_sub(1, std::memory_order::relaxed) > 1) return;

        std::unique_lock<std::mutex> lk(initialized_mutex());
        if(!initialized()) return;
        enet_deinitialize();
        initialized() = false;
    }
}


namespace ggez::impl {
    std::atomic<std::size_t>& instance_count() noexcept {
        static std::atomic<std::size_t> ret{0};
        return ret;
    }


    bool& initialized() noexcept {
        static bool ret{false};
        return ret;
    }

    std::mutex& initialized_mutex() noexcept {
        static std::mutex ret{};
        return ret;
    }
}