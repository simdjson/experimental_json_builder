cmake_minimum_required(VERSION 3.10)

# Set C++ standard to C++17 or higher if needed
#set(CMAKE_CXX_STANDARD 23)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optionally, specify the C++ compiler directly (here we use clang++)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)


option(SIMDJSON_SANITIZE_MEMORY "Sanitize memory" OFF)

if(SIMDJSON_SANITIZE_MEMORY)
  message(STATUS "Setting the memory sanitizer.")
  add_compile_options(
      -fsanitize=memory -fno-sanitize-recover=all
  )
  link_libraries(
      -fsanitize=memory -fno-sanitize-recover=all
  )
endif()

if(NOT CMAKE_BUILD_TYPE)
  if(SIMDJSON_SANITIZE OR SIMDJSON_SANITIZE_UNDEFINED)
    message(STATUS "No build type selected and you have enabled the sanitizer, \
default to Debug. Consider setting CMAKE_BUILD_TYPE.")
    message(STATUS "Setting debug optimization flag to -O1 to help sanitizer.")
    set(CMAKE_CXX_FLAGS_DEBUG "-O1" CACHE STRING "" FORCE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
  else()
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  endif()
endif()

# set(CMAKE_OSX_ARCHITECTURES "arm64")

# if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#   set(CMAKE_C_FLAGS_DEBUG "-g -fsanitize=address,undefined -fno-omit-frame-pointer")
#  set(CMAKE_CXX_FLAGS_DEBUG "-g -fsanitize=address,undefined -fno-omit-frame-pointer")
# else()
#   set(CMAKE_C_FLAGS_RELEASE "-O2 -fsanitize=address,undefined")
#  set(CMAKE_CXX_FLAGS_RELEASE "-O2 -fsanitize=address,undefined")
#  set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -fsanitize=address,undefined")
#  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -fsanitize=address,undefined")
#endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR})

# Add executable targets
add_executable(SerializationBenchmark benchmark_serialization.cpp)
add_executable(SerializationTwitterBenchmark benchmark_serialization_twitter.cpp)

target_compile_options(SerializationBenchmark PRIVATE -freflection -stdlib=libc++ -std=c++26)
target_compile_options(SerializationTwitterBenchmark PRIVATE -freflection -stdlib=libc++ -std=c++26)


# Link sanitizers and libraries
target_link_libraries(SerializationTwitterBenchmark PRIVATE nlohmann_json::nlohmann_json simdjson::simdjson )

target_link_libraries(SerializationBenchmark PRIVATE simdjson-serial nlohmann_json::nlohmann_json simdjson::simdjson)

# cpp-reflect may fail to build on LLVM 19 due to this issue
# https://github.com/hanickadot/compile-time-regular-expressions/issues/307
option(SIMDJSON_BENCH_CPP_REFLECT "include cpp reflect in the benchmarks" OFF)
if(SIMDJSON_BENCH_CPP_REFLECT)
  message(STATUS "Including reflect-cpp in the benchmarks.")
  add_library(benchmark_reflect_serialization_twitter STATIC benchmark_reflect_serialization_twitter.cpp)
  target_link_libraries(benchmark_reflect_serialization_twitter PRIVATE reflectcpp)

  target_link_libraries(SerializationTwitterBenchmark PRIVATE benchmark_reflect_serialization_twitter)
  target_compile_definitions(SerializationTwitterBenchmark PRIVATE SIMDJSON_BENCH_CPP_REFLECT=1)


  add_library(benchmark_reflect_serialization STATIC benchmark_reflect_serialization_twitter.cpp)
  target_link_libraries(benchmark_reflect_serialization PRIVATE reflectcpp)

  target_link_libraries(SerializationBenchmark PRIVATE benchmark_reflect_serialization)
  target_compile_definitions(SerializationBenchmark PRIVATE SIMDJSON_BENCH_CPP_REFLECT=1)
endif()



target_compile_definitions(SerializationTwitterBenchmark PRIVATE JSON_FILE="${CMAKE_CURRENT_SOURCE_DIR}/data/twitter.json")