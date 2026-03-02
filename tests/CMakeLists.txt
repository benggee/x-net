cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_STANDARD 99)

project(x-net-tests)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/../lib ${PROJECT_SOURCE_DIR}/..)

# Test executables
add_executable(test_buffer test_buffer.c ../lib/*.c)
target_link_libraries(test_buffer pthread)

add_executable(test_http test_http.c ../http-server/*.c ../lib/*.c)
target_link_libraries(test_http pthread)

add_executable(test_config test_config.c ../lib/*.c)
target_link_libraries(test_config pthread)

# Benchmark executable
add_executable(benchmark benchmark.c ../http-server/*.c ../lib/*.c)
target_link_libraries(benchmark pthread)

# Enable testing
enable_testing()

# Add tests
add_test(NAME buffer_test COMMAND test_buffer)
add_test(NAME http_test COMMAND test_http)
add_test(NAME config_test COMMAND test_config)
