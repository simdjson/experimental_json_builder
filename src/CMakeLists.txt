cmake_minimum_required(VERSION 3.10) # Set the minimum required version of CMake

project(ExperimentalJsonBuilder) # Name your project

# Set C++ standard to C++17 or higher if needed
#set(CMAKE_CXX_STANDARD 23)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

#if (NOT CMAKE_BUILD_TYPE)
#  message(STATUS "No build type selected, default to Debug")
#  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
#endif()

# Optionally, specify the C++ compiler directly (here we use clang++)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_OSX_ARCHITECTURES "arm64")

# if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#   set(CMAKE_C_FLAGS_DEBUG "-g -fsanitize=address,undefined -fno-omit-frame-pointer")
#   set(CMAKE_CXX_FLAGS_DEBUG "-g -fsanitize=address,undefined -fno-omit-frame-pointer")
# else()
#   set(CMAKE_C_FLAGS_RELEASE "-O2 -fsanitize=address,undefined")
#   set(CMAKE_CXX_FLAGS_RELEASE "-O2 -fsanitize=address,undefined")
#   set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -fsanitize=address,undefined")
#   set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -fsanitize=address,undefined")
# endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR})

# Add executable target
add_executable(ExperimentalJsonBuilder ../examples/example.cpp)
add_executable(ExperimentalDeserialization ../examples/example2.cpp)

add_library(simdjson-serial INTERFACE)
target_include_directories(simdjson-serial INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_compile_options(ExperimentalJsonBuilder PRIVATE -freflection -stdlib=libc++ -std=c++26)
target_compile_options(ExperimentalDeserialization PRIVATE -freflection -stdlib=libc++ -std=c++26)

# If you have any libraries to link (e.g., if your project grows and you decide to compile some parts into a library),
# you would specify them here with target_link_libraries(MyLibraryExample mylib)
