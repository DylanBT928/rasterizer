cmake_minimum_required(VERSION 3.24)

get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT is_multi_config AND NOT (CMAKE_BUILD_TYPE OR DEFINED ENV{CMAKE_BUILD_TYPE}))
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Release default")
endif()

project(rasterizer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU|Intel")
    add_compile_options(-Wall)
endif()

find_package(OpenMP COMPONENTS CXX)

file(GLOB SOURCES "src/*.cpp")

add_executable(rasterizer ${SOURCES})
target_link_libraries(rasterizer PRIVATE $<$<BOOL:${OpenMP_CXX_FOUND}>:OpenMP::OpenMP_CXX>)
