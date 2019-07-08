cmake_minimum_required(VERSION 3.14)
project(multidns)

set(CMAKE_CXX_STANDARD 98)

add_executable(multidns main.cpp queue.c dns_headers.h)

target_link_libraries(multidns pthread)