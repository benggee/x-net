cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_STANDARD 99)

project(http-server)

include(CheckSymbolExists)
check_symbol_exists(epoll_create "sys/epoll.h" EPOLL_EXISTS)
if (EPOLL_EXISTS)
    set(EPOLL_ENABLE 1 CACHE INTERNAL "enable epoll")
else()
    set(EPOLL_ENABLE "" CACHE INTERNAL "not enable epoll")
endif()

configure_file(config.h.cmake ${PROJECT_SOURCE_DIR}/../lib/config.h)

file(GLOB src_files ../lib/*.c *.c)

message(${src_files})

include_directories(${PROJECT_SOURCE_DIR}/../lib ${PROJECT_SOURCE_DIR})

add_executable(http-server ${src_files})

target_link_libraries(http-server pthread)