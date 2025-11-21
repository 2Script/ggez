#include "./session_types.hpp"
#include "./debug_callbacks.hpp"
#include "./test_data.hpp"

#include <future>



int main() {
    ggez::test::host local_host{ggez::any_host, ggez::test::port, ggez::test::metadata_t{}, ggez::test::debug::host_callbacks};
    RESULT_VERIFY(local_host.initialize());

    ggez::test::client local_player{ggez::client_model::player, ggez::test::metadata_t{}, ggez::test::debug::client_callbacks};
    RESULT_VERIFY(local_player.initialize());

    std::future<ggez::result<void>> poll_host_async = std::async([&local_host]() -> ggez::result<void> {
        auto r = local_host.poll_network(ggez::session::default_timeout_ms * 10);
        ggez::test::debug::print_host_line("Done polling: {}", r.has_value() ? "Success" : std::to_string(static_cast<std::size_t>(r.error())));
        return r;
    });
    RESULT_VERIFY(local_player.connect_local(local_host));

    std::future<ggez::result<void>> poll_player_async = std::async([&local_player]() -> ggez::result<void> {    
        auto r = local_player.poll_network();
        ggez::test::debug::print_client_line("Done polling: {}", r.has_value() ? "Success" : std::to_string(static_cast<std::size_t>(r.error())));
        return r;
    });

    ggez::test::debug::print_client_line("Waiting for host ackowledgement...");
    local_player.wait_until_acknowledged_by_host();

    ggez::test::debug::print_client_line("Waiting for 2 players to connect...");
    local_player.wait_until_enough_players_connected();

    ggez::test::step_t test_step = 0;
    ggez::test::debug::print_client_line("Sending local per step data (#{}) for step #{}", 0, test_step);
    RESULT_VERIFY(local_player.send_local_per_step_data(ggez::test::per_step_data[0], test_step));
    local_player.push_back_local(ggez::test::per_step_data[0], test_step);


    ggez::test::debug::print_client_line("Waiting for all per step data...");
    local_player.wait_until_no_mismatch();
    std::this_thread::sleep_for(std::chrono::seconds(10));


    auto state = local_player.front();
    if(std::memcmp(&state[0], &ggez::test::per_step_data[0], sizeof(ggez::test::per_step_data_t)) != 0) 
        ggez::test::debug::print_client_line("per step data does not match for {} client", "local");

    if(std::memcmp(&state[1], &ggez::test::per_step_data[1], sizeof(ggez::test::per_step_data_t)) != 0) 
        ggez::test::debug::print_client_line("per step data does not match for {} client", "remote");
    
    
    RESULT_VERIFY(poll_host_async.get());
    RESULT_VERIFY(poll_player_async.get());
    return 0;
}