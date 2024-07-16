cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
project(experimental_json_builder
    VERSION 0.0.1
    DESCRIPTION "Serializing gigabytes of JSON per second"
    HOMEPAGE_URL "https://simdjson.org/"
    LANGUAGES CXX
)



option(SIMDJSON_SERIALIZER_SANITIZE_MEMORY "Sanitize memory" OFF)

if(SIMDJSON_SERIALIZER_SANITIZE_MEMORY)
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

if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

add_library(simdjson_serialization INTERFACE)
add_library(simdjson::serialization ALIAS simdjson_serialization)
target_compile_options(simdjson_serialization INTERFACE  -freflection -stdlib=libc++ -std=c++26)

target_include_directories(
  simdjson_serialization
  INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

include(cmake/CPM.cmake)
# include simdjson
CPMAddPackage("gh:simdjson/simdjson@3.9.4")


add_subdirectory(examples)
add_subdirectory(benchmarks)

enable_testing()
add_subdirectory(tests)
