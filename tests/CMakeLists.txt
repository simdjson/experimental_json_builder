add_compile_options(-Wall -Wextra -Weffc++ -Wsign-compare -Wshadow -Wwrite-strings -Wpointer-arith -Winit-self -Wconversion -Wno-sign-conversion)

add_executable(user_profile_tests user_profile_tests.cpp)
target_link_libraries(user_profile_tests nlohmann_json::nlohmann_json simdjson::serialization simdjson::simdjson)
add_test(user_profile_tests user_profile_tests)