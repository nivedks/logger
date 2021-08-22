cmake_minimum_required(VERSION 3.5)

set(CMAKE_PROJECT_TARGET Logger)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../inc/
    /usr/local/include/
    /usr/include/
)

add_executable(${CMAKE_PROJECT_TARGET} main.cpp Logger.cpp)