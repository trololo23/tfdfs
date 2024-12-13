cmake_minimum_required(VERSION 3.10)
project(IntegralCalculator)

set(CMAKE_CPP_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O2")

add_executable(server server.cpp)
add_executable(client client.cpp)

configure_file(${CMAKE_SOURCE_DIR}/scripts/test_network.sh
               ${CMAKE_BINARY_DIR}/test_network.sh COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/scripts/run_tests.sh
               ${CMAKE_BINARY_DIR}/run_tests.sh COPYONLY) 