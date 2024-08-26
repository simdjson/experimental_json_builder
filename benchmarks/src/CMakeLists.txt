if(SIMDJSON_SERIALIZER_SANITIZE_MEMORY)
  message(STATUS "Setting the memory sanitizer.")
  add_compile_options(
      -fsanitize=memory -fno-sanitize-recover=all
  )
  link_libraries(
      -fsanitize=memory -fno-sanitize-recover=all
  )
endif()

#include_directories(${CMAKE_SOURCE_DIR})

# Add executable targets
add_executable(SerializationBenchmark benchmark_serialization.cpp)
add_executable(SerializationTwitterBenchmark benchmark_serialization_twitter.cpp)

 if(TARGET serde-benchmark)
    message(STATUS "serde-benchmark target was created. Linking benchmarks and serde-benchmark.")
    target_link_libraries(SerializationTwitterBenchmark PRIVATE serde-benchmark)
    target_compile_definitions(SerializationTwitterBenchmark PRIVATE SIMDJSON_RUST_VERSION="${Rust_VERSION}")
endif()
target_link_libraries(SerializationTwitterBenchmark PRIVATE simpleparser simdjson::serialization nlohmann_json::nlohmann_json simdjson::simdjson)
target_link_libraries(SerializationBenchmark PRIVATE simpleparser simdjson::serialization nlohmann_json::nlohmann_json simdjson::simdjson)

# cpp-reflect may fail to build on LLVM 19 due to this issue
# https://github.com/hanickadot/compile-time-regular-expressions/issues/307
option(SIMDJSON_BENCH_CPP_REFLECT "include cpp reflect in the benchmarks" ON)
if(SIMDJSON_BENCH_CPP_REFLECT)
  message(STATUS "Including reflect-cpp in the benchmarks.")
  add_library(benchmark_reflect_serialization_twitter STATIC benchmark_reflect_serialization_twitter.cpp)
  target_link_libraries(benchmark_reflect_serialization_twitter PRIVATE reflectcpp)

  target_link_libraries(SerializationTwitterBenchmark PRIVATE benchmark_reflect_serialization_twitter)
  target_compile_definitions(SerializationTwitterBenchmark PRIVATE SIMDJSON_BENCH_CPP_REFLECT=1)


  add_library(benchmark_reflect_serialization STATIC benchmark_reflect_serialization.cpp)
  target_link_libraries(benchmark_reflect_serialization PRIVATE reflectcpp)

  target_link_libraries(SerializationBenchmark PRIVATE benchmark_reflect_serialization)
  target_compile_definitions(SerializationBenchmark PRIVATE SIMDJSON_BENCH_CPP_REFLECT=1)
endif()



target_compile_definitions(SerializationTwitterBenchmark PRIVATE JSON_FILE="${CMAKE_CURRENT_SOURCE_DIR}/data/twitter.json")