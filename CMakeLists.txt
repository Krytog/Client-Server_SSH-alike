cmake_minimum_required(VERSION 3.16)
project(Client-Server_App C)

add_subdirectory(Network)

add_executable(Client Client/client.c Client/utilities.c)
add_executable(Server Server/server.c Server/utilities.c)

target_link_libraries(Client PRIVATE network pthread)
target_link_libraries(Server PRIVATE network pthread)