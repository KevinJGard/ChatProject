cmake_minimum_required(VERSION 3.11)
project(ChatProject)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare (
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

FetchContent_MakeAvailable(nlohmann_json)

add_executable(client 
    src/model/client.cpp
    src/controller/client_controller.cpp
    src/view/client_view.cpp
    src/client_main.cpp)
add_executable(server 
    src/model/server.cpp
    src/controller/server_controller.cpp
    src/view/server_view.cpp
    src/server_main.cpp)

target_link_libraries(client PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(server PRIVATE nlohmann_json::nlohmann_json)