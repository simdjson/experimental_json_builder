add_compile_options(-Wall -Wextra -Weffc++ -Wsign-compare -Wshadow -Wwrite-strings
-Wpointer-arith -Winit-self -Wconversion -Wno-sign-conversion)

add_executable(example example.cpp)
target_link_libraries(example PRIVATE simdjson::serialization simdjson::simdjson)

add_executable(example2 example2.cpp)
target_link_libraries(example2 PRIVATE simdjson::serialization simdjson::simdjson)
