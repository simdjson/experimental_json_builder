#include "simdjson.h"
#include "simdjson/json_builder/json_builder.h"
#include <cstdlib>
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


///
/// For simplicity, the definitions of invoke should be non-overlapping.
/// If we provide both a specialization for a container and a specialization for a user-defined type,
/// which one gets picked is maybe unclear???
/// 
template <typename T>
  requires (json_builder::UserDefinedType<T> && ! PushableContainer<T>)
simdjson_result<T>
tag_invoke(deserialize_tag, std::type_identity<T>, ondemand::value &val) {
  ondemand::object obj;
  auto error = val.get_object().get(obj);
  if (error) {
    return error;
  }
  T t;
  [:json_builder::expand(std::meta::nonstatic_data_members_of(^T)):] >> [&]<auto mem> {
    auto it = obj[(std::string_view)std::meta::identifier_of(mem)];
    using MemberType = typename std::remove_reference_t<decltype(t.[:mem:])>;

    if constexpr (std::is_same_v<MemberType, int>) {
      int64_t val;
      if(auto err = it.get(val); err) { return err; }
      t.[:mem:] = static_cast<int>(val);
    } else if constexpr (std::is_same_v<MemberType, std::string>) {
      if(auto err = it.get_string(t.[:mem:]); err) { return err; }
    } else {
      if(auto err = it.get(t.[:mem:]); err) { return err; }
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
  std::vector<int> result;
  if(auto err = doc["values"].get<std::vector<int>>().get(result); err) {
    std::cerr << "Error parsing JSON: " << simdjson::error_message(err) << std::endl;
    return EXIT_FAILURE;
  }
  for (const int x : result) {
    std::cout << x << std::endl;
  }

  /////////////
  /// Notice how we won't parse directly the *document* but a value inside the document.
  /// That's because the document is not an instance of a value in simdjson, and so the
  /// current patch would require a bit of extra work to make it work with both values
  /// and documents
  //////////////
  std::string json_str3 = R"({"toto":{"id": 1, "name": "example", "values": [4, 42, 43]}})";
  ondemand::parser parser3;
  auto doc3 = parser3.iterate(json_str3);

  MyStruct result3;
  if(auto err = doc3["toto"].get(result3); err) {
    std::cerr << "Error parsing JSON: " << simdjson::error_message(err) << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "ID: " << result3.id << ", Name: " << result3.name ;
  for (const int x : result3.values) {
    std::cout << x << std::endl;
  }
  return EXIT_SUCCESS;
}
