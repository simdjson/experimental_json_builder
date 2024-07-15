cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
add_compile_options(-Wall -Wextra -freflection -stdlib=libc++ -std=c++26)
project(experimental_json_builder
    VERSION 0.0.1
    DESCRIPTION "Serializing gigabytes of JSON per second"
    HOMEPAGE_URL "https://simdjson.org/"
    LANGUAGES CXX
)
if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()
#set(CMAKE_CXX_STANDARD 23)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/CPM.cmake)
# include simdjson
CPMAddPackage("gh:simdjson/simdjson@3.9.4")
CPMAddPackage("gh:nlohmann/json@3.10.5")

# Include reflect-cpp
CPMAddPackage(
  NAME reflect-cpp
  GITHUB_REPOSITORY getml/reflect-cpp
  GIT_TAG v0.13.0
)

# Optionally, specify the C++ compiler directly (here we use clang++)
add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(benchmarks/src)

enable_testing()
add_subdirectory(tests)
