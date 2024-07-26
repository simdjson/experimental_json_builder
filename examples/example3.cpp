#include "simdjson.h"
#include <string>
#include <vector>
#include <iostream>
#include <type_traits>

using namespace simdjson;

namespace simdjson {

template <typename T>
concept PushableContainer =
    requires(T a, typename T::value_type val) {
      a.push_back(val);
    } && !std::is_same_v<T, std::string> &&
    !std::is_same_v<T, std::string_view> && 
    !std::is_same_v<T, const char*>;

// Specialize tag_invoke for std::vector<int>
template <typename T>
  requires PushableContainer<T>
simdjson_result<T>
tag_invoke(deserialize_tag, std::type_identity<T>, ondemand::value &val) {
  T vec;
  auto array_result = val.get_array();
  if (array_result.error()) return array_result.error();
  ondemand::array array = array_result.value();

  for (auto v : array) {
    typename T::value_type element;
    if constexpr (std::is_same_v<typename T::value_type, int>) {
      auto int_result = v.get_int64();
      if (int_result.error()) return int_result.error();
      element = static_cast<int>(int_result.value());
    } else if constexpr (std::is_same_v<typename T::value_type, double>) {
      auto double_result = v.get_double();
      if (double_result.error()) return double_result.error();
      element = double_result.value();
    } else if constexpr (std::is_same_v<typename T::value_type, std::string>) {
      auto string_result = v.get_string();
      if (string_result.error()) return string_result.error();
      element = std::string(string_result.value());
    } else if constexpr (std::is_same_v<typename T::value_type, bool>) {
      auto bool_result = v.get_bool();
      if (bool_result.error()) return bool_result.error();
      element = bool_result.value();
    } else {
      // Unsupported type
      static_assert(!std::is_same_v<T, T>, "Unsupported type in JSON array");
    }
    vec.push_back(std::move(element));
  }

  return vec;
}

} // namespace simdjson

int main() {
  std::string json_str = R"({"values": [1, 2, 3]})";
  ondemand::parser parser;
  auto doc = parser.iterate(json_str);

  auto result = doc["values"].get<std::vector<int>>();
  if (result.error()) {
    std::cerr << "Error parsing JSON: " << result.error() << std::endl;
    return 1;
  }

  std::vector<int> my_struct = result.value();
  for (const int &x : my_struct) {
    std::cout << x << std::endl;
  }

  return 0;
}
