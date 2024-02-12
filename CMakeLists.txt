cmake_minimum_required(VERSION 3.0)
project(clock_stuff)

set(CMAKE_CXX_STANDARD 17)

find_package(thallium REQUIRED)
if(thallium_FOUND)
    message(STATUS "found thallium at ${thallium_DIR}")
endif()

find_package(cereal REQUIRED)

add_subdirectory(network_latency)
add_subdirectory(time_serialization)
add_subdirectory(ltrace)