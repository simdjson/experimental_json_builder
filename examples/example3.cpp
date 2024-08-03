#include "simdjson.h"
#include "simdjson/json_builder/json_builder.h"
#include <string>
#include <vector>
#include <iostream>
#include <type_traits>

using namespace simdjson;

// Example user-defined type
struct MyStruct {
  int id;
  std::string name;
  std::vector<int> values;
};

// Example of simple user-defined type
struct MySimpleStruct {
  int id;
  std::string name;
};

namespace simdjson {

template <typename T>
concept PushableContainer =
    requires(T a, typename T::value_type val) {
      a.push_back(val);
    } && !std::is_same_v<T, std::string> &&
    !std::is_same_v<T, std::string_view> && 
    !std::is_same_v<T, const char*>;

// Specialize tag_invoke for containers
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

template <typename T>
  requires json_builder::UserDefinedType<T>
simdjson_result<T>
tag_invoke(deserialize_tag, std::type_identity<T>, ondemand::value &val) {
  ondemand::object obj;
  auto error = val.get_object().get(obj);
  if (error) {
    return error;
  }
  T t;
  [:json_builder::expand(std::meta::nonstatic_data_members_of(^T)):] >> [&]<auto mem> {
    auto it = obj[to_string_view(std::meta::identifier_of(mem))];
    using MemberType = typename std::remove_reference_t<decltype(t.[:mem:])>;

    if constexpr (std::is_same_v<MemberType, int>) {
      auto int_result = it.get_int64();
      if (int_result.error()) throw std::runtime_error("Error getting int64");
      t.[:mem:] = static_cast<int>(int_result.value());
    } else if constexpr (std::is_same_v<MemberType, double>) {
      auto double_result = it.get_double();
      if (double_result.error()) throw std::runtime_error("Error getting double");
      t.[:mem:] = double_result.value();
    } else if constexpr (std::is_same_v<MemberType, std::string>) {
      auto string_result = it.get_string();
      if (string_result.error()) throw std::runtime_error("Error getting string");
      t.[:mem:] = std::string(string_result.value());
    } else if constexpr (std::is_same_v<MemberType, bool>) {
      auto bool_result = it.get_bool();
      if (bool_result.error()) throw std::runtime_error("Error getting bool");
      t.[:mem:] = bool_result.value();
    } else {
      t.[:mem:] = it.template get<MemberType>().value();
    }
  };
  return t;
}

} // simdjson

int main() {
  // Test for std::vector<int>
  std::string json_str = R"({"values": [1, 2, 3]})";
  ondemand::parser parser;
  auto doc = parser.iterate(json_str);

  auto result = doc["values"].get<std::vector<int>>();
  if (result.error()) {
    std::cerr << "Error parsing JSON: " << result.error() << std::endl;
    return 1;
  }

  std::vector<int> z = result.value();
  for (const int &x : z) {
    std::cout << x << std::endl;
  }

  // Test for user-defined type MyStruct
  std::string json_str3 = R"({"id": 1, "name": "example", "values": [1, 2, 3]})";
  ondemand::parser parser3;
  auto doc3 = parser3.iterate(json_str3);

  auto result3 = doc3.get<MyStruct>();
  if (result3.error()) {
    std::cerr << "Error parsing JSON: " << result3.error() << std::endl;
    return 1;
  }

  MyStruct my_struct = result3.value();
  std::cout << "ID: " << my_struct.id << ", Name: " << my_struct.name << ", Values: ";
  for (const int &x : my_struct.values) {
    std::cout << x << " ";
  }
  std::cout << std::endl;
  return 0;
}
