cmake_minimum_required(VERSION 3.15)

project(ggez VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")



### sources ###
set(CPP_FILES

)
list(TRANSFORM CPP_FILES PREPEND src/${PROJECT_NAME}/)



### targets ###
add_library(${PROJECT_NAME} ${CPP_FILES})


### compiler args ###
list(APPEND CLANG_ARGS "-Wall" "-Wextra" "-fno-exceptions" "-fno-rtti")

target_compile_options(${PROJECT_NAME} PUBLIC ${CLANG_ARGS})
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:-Werror>")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:-O0>")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:-g>")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:-ftemplate-backtrace-limit=0>")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:-O3>")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:-ffast-math>")
#target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:-march=native>")
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang") # clang only
    #target_compile_options(${PROJECT_NAME} PUBLIC "-Wc++20-compat-pedantic")
    target_compile_options(${PROJECT_NAME} PUBLIC "-fsized-deallocation")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.1.0")
        # Don't show warnings for C++20 designated field initializers.
        target_compile_options(${PROJECT_NAME} PUBLIC "-Wno-missing-designated-field-initializers")
    else()
        # If clang is "outdated", then we have to disable the entire warning category
        target_compile_options(${PROJECT_NAME} PUBLIC "-Wno-missing-field-initializers")
    endif()
else() # gcc only
    #target_compile_options(${PROJECT_NAME} PUBLIC "-Wc++20-compat")
    # gcc has no "missing-designated-field-initializers" (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=96868)
    target_compile_options(${PROJECT_NAME} PUBLIC "-Wno-missing-field-initializers")
endif()

### clang compile commands ###
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


### dependencies ###
target_link_directories(${PROJECT_NAME} PUBLIC "/usr/local/lib" "/usr/lib")



### self include directory ###
cmake_path(SET SELF_INC_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
file(TO_NATIVE_PATH "${SELF_INC_CMAKE_DIR}" SELF_INC_DIR)
target_include_directories(${PROJECT_NAME} BEFORE PUBLIC "${SELF_INC_DIR}")
message(STATUS "Including self directory: ${SELF_INC_DIR}")


### install ###
install(FILES "$<TARGET_FILE:${PROJECT_NAME}>" TYPE LIB CONFIGURATIONS Release)
install(FILES "$<TARGET_FILE:${PROJECT_NAME}>" DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/debug/usr/lib" CONFIGURATIONS Debug RENAME "$<TARGET_FILE_NAME:${PROJECT_NAME}>.debug")
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY src/ DESTINATION src)

