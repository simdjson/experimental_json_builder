cmake_minimum_required(VERSION 3.10)
project(experimental_json_builder)

# Set C++ standard to C++17 or higher if needed
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optionally, specify the C++ compiler directly (here we use clang++)
add_subdirectory(src)
add_subdirectory(benchmarks/src)
