cmake_minimum_required(VERSION 3.10)

# Set the project name
project(LAB08)

# Use C11
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include directories
include_directories(includes)

# Source files for server
set(SERVER_SOURCES
    src/server.c
    src/utils.c
    src/client_handler.c
)

# Source files for client
set(CLIENT_SOURCES
    src/client.c
    src/utils.c
    src/client_func.c
)

# Create executables
add_executable(server ${SERVER_SOURCES})
add_executable(client ${CLIENT_SOURCES})
