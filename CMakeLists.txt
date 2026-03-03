cmake_minimum_required(VERSION 3.15)
project(mizuna)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)

add_library(mizuna)

add_subdirectory(lib/mizuna)
add_subdirectory(lib/hk)
add_subdirectory(lib/tinygltf)
