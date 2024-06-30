cmake_minimum_required(VERSION 3.10)

# Project name
project(TCPClientServer)

# C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add server executable
add_executable(server server.cpp)

# Add client executable
add_executable(client client.cpp)

# Link pthread library
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(server Threads::Threads)
target_link_libraries(client Threads::Threads)
