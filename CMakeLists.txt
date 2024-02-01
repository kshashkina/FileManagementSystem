cmake_minimum_required(VERSION 3.26)
project(csc_first)

set(CMAKE_CXX_STANDARD 17)

add_executable(server server.cpp)
target_link_libraries(server PRIVATE ws2_32)

add_executable(client1 client1.cpp)
target_link_libraries(client1 PRIVATE ws2_32)


add_executable(client2 client2.cpp)
target_link_libraries(client2 PRIVATE ws2_32)