cmake_minimum_required(VERSION 3.26)
project(csc_first)

set(CMAKE_CXX_STANDARD 17)

# Add the server executable
add_executable(server server.cpp)
target_link_libraries(server PRIVATE ws2_32)

# Add the client1 executable with both client1.cpp and Client.cpp
add_executable(client1 client1.cpp Client.cpp client.h)
target_link_libraries(client1 PRIVATE ws2_32)

# Add the client2 executable with both client2.cpp and Client.cpp
add_executable(client2 client2.cpp Client.cpp client.h)
target_link_libraries(client2 PRIVATE ws2_32)
