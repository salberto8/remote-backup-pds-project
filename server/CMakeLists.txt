cmake_minimum_required(VERSION 3.15)
project(server)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)

add_executable(server
        authorization.cpp
        authorization.h
        backup.cpp
        backup.h
        CMakeLists.txt
        Listener.cpp
        Listener.h
        main.cpp
        server.cpp
        server.h
        Session.h dao.h configuration.cpp configuration.h dao.cpp Session.cpp)


set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(server Threads::Threads crypto boost_program_options sqlite3 stdc++fs)

