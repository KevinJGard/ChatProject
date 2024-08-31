cmake_minimum_required(VERSION 3.10)
project(Chat)

set(CMAKE_CXX_STANDARD 17)

# Add source files
file(GLOB_RECURSE SOURCES "src/**/*.cpp")

add_executable(ChatProject ${SOURCES})