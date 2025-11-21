cmake_minimum_required(VERSION 3.15)

project(ggez VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")


### sources ###
set(${PROJECT_NAME}_HEADER_FILES
    "aux/byte_swap.hpp"
    "aux/client_id_set.hpp"
    "aux/enet_host_deleter.hpp"
    "aux/from_bytes.hpp"
    "aux/instance_tracker.hpp"
    "aux/instance_tracker.inl"
    #"aux/start_lifetime_as.hpp"

    "core/client_id_t.hpp"
    "core/errc.hpp"
    "core/result.hpp"

    "infra/byte_order.hpp"
    "infra/client_counts_t.hpp"
    "infra/client_info.hpp"
    "infra/client_model.hpp"
    "infra/data_source.hpp"
    "infra/disconnect_reason.hpp"
    "infra/discrepancy_policy.hpp"
    "infra/endpoint.hpp"
    "infra/host_info.hpp"
    "infra/ip_address.hpp"
    "infra/ip_address.inl"
    "infra/network_address_view.hpp"
    "infra/network_data_id.hpp"
    #"infra/network_data_offset.hpp"
    "infra/network_data_policy.hpp"
    "infra/network_port.hpp"
    "infra/network_stats_t.hpp"
    "infra/step_info.hpp"
    
    "session/basic_base.hpp"
    "session/basic_base.inl"
    "session/basic_client.hpp"
    "session/basic_client.inl"
    "session/basic_host.hpp"
    "session/basic_host.inl"
    "session/basic.hpp"
    "session/callbacks.hpp"
    "session/config_t.hpp"
    "session/function.hpp"
    "session/metadata_t.hpp"
    "session/model.hpp"

    "traits/config_like.hpp"
    "traits/has_valid_client_count_upper_limits.hpp"
    "traits/is_implicit_lifetime.hpp"
    "traits/is_implicitly_creatable_by_copy.hpp"
    "traits/is_network_transferrable.hpp"
    "traits/metadata_like.hpp"
)



### targets ###
add_library(${PROJECT_NAME} INTERFACE)
list(TRANSFORM ${PROJECT_NAME}_HEADER_FILES PREPEND "include/${PROJECT_NAME}/")
target_sources(${PROJECT_NAME} PUBLIC FILE_SET ${PROJECT_NAME}_HEADERS TYPE HEADERS BASE_DIRS "include" FILES ${${PROJECT_NAME}_HEADER_FILES})


### self include directory ###
cmake_path(SET SELF_INC_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
file(TO_NATIVE_PATH "${SELF_INC_CMAKE_DIR}" SELF_INC_DIR)
target_include_directories(${PROJECT_NAME} SYSTEM BEFORE INTERFACE "${SELF_INC_DIR}")
message(STATUS "Including self directory: ${SELF_INC_DIR}")


### test executables ###
add_subdirectory(test)

### install ###
install(TARGETS ${PROJECT_NAME} FILE_SET ${PROJECT_NAME}_HEADERS)
