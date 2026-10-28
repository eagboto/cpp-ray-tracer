cmake_minimum_required(VERSION 3.16)
project(RayTracer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build optimized by default — a debug build renders many times slower.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Threads REQUIRED)

add_executable(raytracer src/main.cpp)
target_compile_options(raytracer PRIVATE -Wall -Wextra)
target_link_libraries(raytracer PRIVATE Threads::Threads)
