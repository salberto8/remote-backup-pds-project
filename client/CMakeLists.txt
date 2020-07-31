cmake_minimum_required(VERSION 3.17)
project(client)

set(CMAKE_CXX_STANDARD 17)

add_executable(client main.cpp configuration.cpp configuration.h FileWatcher.cpp FileWatcher.h Client.cpp Client.h)

find_package(Threads REQUIRED)
target_link_libraries(client Threads::Threads boost_program_options)