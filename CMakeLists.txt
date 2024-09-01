cmake_minimum_required(VERSION 3.10)
project(SocketExample)

set(CMAKE_CXX_STANDARD 11)

include_directories(${CMAKE_SOURCE_DIR})

add_executable(client src/model/client.cpp)

add_executable(server src/model/server.cpp)