cmake_minimum_required(VERSION 3.16)
project(chat_cli LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

add_executable(chat_cli chat_cli.cpp)
target_include_directories(chat_cli PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(chat_cli PRIVATE ${Boost_LIBRARIES} Threads::Threads)
