set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

add_executable(user_profile_tests user_profile_tests.cpp)
target_compile_options(user_profile_tests PRIVATE -freflection -stdlib=libc++ -std=c++26)
target_link_libraries(user_profile_tests nlohmann_json::nlohmann_json simdjson-serial simdjson::simdjson)

add_test(user_profile_tests user_profile_tests)