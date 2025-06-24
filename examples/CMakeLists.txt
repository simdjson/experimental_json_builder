add_compile_options(-Wall -Wextra -Weffc++ -Wsign-compare -Wshadow -Wwrite-strings
-Wpointer-arith -Winit-self -Wconversion -Wno-sign-conversion)

add_executable(demo demo.cpp)
target_link_libraries(demo PRIVATE simdjson::serialization simdjson::simdjson)

add_executable(example example.cpp)
target_link_libraries(example PRIVATE simdjson::serialization simdjson::simdjson)

add_executable(example2 example2.cpp)
target_link_libraries(example2 PRIVATE simdjson::serialization simdjson::simdjson)

add_executable(example3 example3.cpp)
target_link_libraries(example3 PRIVATE simdjson::serialization simdjson::simdjson)

add_executable(example4 example4.cpp)
target_link_libraries(example4 PRIVATE simdjson::serialization)