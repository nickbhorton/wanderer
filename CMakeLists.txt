cmake_minimum_required(VERSION 3.29)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED On)

project(
    wanderer
    LANGUAGES CXX
)

add_library(
    udp_server 
    src/udp_server.cc
)
