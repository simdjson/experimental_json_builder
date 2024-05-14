cmake_minimum_required(VERSION 3.10) # Set the minimum required version of CMake

project(SerializationBenchmark) # Name your project
if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()
# Set C++ standard to C++17 or higher if needed
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optionally, specify the C++ compiler directly (here we use clang++)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

# Include directories
include_directories(${CMAKE_SOURCE_DIR})

# Add executable target
add_executable(SerializationBenchmark benchmark_serialization.cpp)
target_link_libraries(SerializationBenchmark PRIVATE simdjson-serial)

target_compile_options(SerializationBenchmark PRIVATE -freflection -stdlib=libc++ -std=c++26)
# If you have any libraries to link (e.g., if your project grows and you decide to compile some parts into a library),
# you would specify them here with target_link_libraries(MyLibraryExample mylib)
