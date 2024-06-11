cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

project(experimental_json_builder
    VERSION 0.0.1
    DESCRIPTION "Serializing gigabytes of JSON per second"
    HOMEPAGE_URL "https://simdjson.org/"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/CPM.cmake)
# include simdjson
CPMAddPackage("gh:simdjson/simdjson@3.9.4")
CPMAddPackage("gh:nlohmann/json@3.10.5")

# Optionally, specify the C++ compiler directly (here we use clang++)
add_subdirectory(src)
add_subdirectory(benchmarks/src)

enable_testing()
add_subdirectory(tests)
