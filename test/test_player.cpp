#include "./session_types.hpp"
#include "./debug_callbacks.hpp"
#include "./test_data.hpp"

#include <future>


int main() {
    ggez::test::client p{ggez::client_model::player, ggez::test::metadata_t{}, ggez::test::debug::client_callbacks};
    RESULT_VERIFY(p.initialize());

    RESULT_VERIFY(p.connect(ggez::localhost, ggez::test::port));

    std::future<ggez::result<void>> poll_player_async = std::async([&p]() -> ggez::result<void> {
        auto r = p.poll_network();
        ggez::test::debug::print_client_line("Done polling: {}", r.has_value() ? "Success" : std::to_string(static_cast<std::size_t>(r.error())));
        return r;
    });

    ggez::test::debug::print_client_line("Waiting for host ackowledgement...");
    p.wait_until_acknowledged_by_host();

    ggez::test::debug::print_client_line("Waiting for 2 players to connect...");
    p.wait_until_enough_players_connected();


    ggez::test::step_t test_step = 0;
    ggez::test::debug::print_client_line("Sending local per step data (#{}) for step #{}", 1, test_step);
    RESULT_VERIFY(p.send_local_per_step_data(ggez::test::per_step_data[1], test_step));
    p.push_back_local(ggez::test::per_step_data[1], test_step);


    ggez::test::debug::print_client_line("Waiting for all per step data...");
    p.wait_until_no_mismatch();

    auto state = p.front();
    if(std::memcmp(&state[0], &ggez::test::per_step_data[0], sizeof(ggez::test::per_step_data_t)) != 0) 
        ggez::test::debug::print_client_line("per step data does not match for {} client", "remote");

    if(std::memcmp(&state[1], &ggez::test::per_step_data[1], sizeof(ggez::test::per_step_data_t)) != 0) 
        ggez::test::debug::print_client_line("per step data does not match for {} client", "local");


    RESULT_VERIFY(poll_player_async.get());
    return 0;

}