cmake_minimum_required(VERSION 3.0.0)
project(raytracer VERSION 0.1.0)

add_compile_options(-std=c++17 -O3 -fopenmp)
add_executable(raytracer main.cpp)
set(CMAKE_EXE_LINKER_FLAGS  -fopenmp)
message("${CMAKE_EXE_LINKER_FLAGS}")