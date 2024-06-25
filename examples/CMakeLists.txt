set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_OSX_ARCHITECTURES "arm64")
add_executable(example example.cpp)
target_link_libraries(example PRIVATE simdjson-serial)

add_executable(example2 example2.cpp)
target_link_libraries(example2 PRIVATE simdjson-serial)

target_compile_options(example PRIVATE -freflection -stdlib=libc++ -std=c++26)
target_compile_options(example2 PRIVATE -freflection -stdlib=libc++ -std=c++26)
target_link_libraries(example PRIVATE simdjson::simdjson)
