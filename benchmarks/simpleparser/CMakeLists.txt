add_library(simpleparser INTERFACE)
target_include_directories(simpleparser INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(simpleparser INTERFACE simdjson::serialization)