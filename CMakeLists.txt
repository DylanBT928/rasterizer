cmake_minimum_required(VERSION 3.24)

project(katzu LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-O3 -Wall -Wextra -Wpedantic)

file(GLOB SOURCES "src/*.cpp")

add_executable(katzu ${SOURCES})

target_link_libraries(katzu m)
