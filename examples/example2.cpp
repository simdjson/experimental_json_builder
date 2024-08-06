// Note: this code is not yet working and it is temporarily removed from the cmake build
// For this to work, further changes might be needed to simdjson project. For now reflection and invoke_tag only work for ondemand::value::get()

#include "simdjson.h"
#include "simdjson/json_builder/json_builder.h"
#include "simdjson/json_builder/string_builder.h"
#include <format>
#include <iostream>
#include <print>
#include <string>
#include <type_traits>
#include <vector>
using namespace simdjson;

struct MyStruct {
  int id;
  std::string name;
  std::vector<int> values;
};

struct Y {
  int g;
  std::string h;
  std::vector<int> i;
};

struct X {
  char a;
  int b;
  int c;
  std::string d;
  std::vector<int> e;
  std::vector<std::string> f;
  Y y;
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

void demo() {
  std::vector<std::string> vec;
  std::string json_str = R"(["a", "b", "c"])";
  ondemand::parser parser;
  ondemand::document doc = parser.iterate(json_str);
  std::vector<std::string> result = doc.get<std::vector<std::string>>();
  for (auto x : result) {
    std::cout << x << std::endl;
  }
}

int main() {
  demo();

  std::string json_str = R"({"id": 1, "name": "example", "values": [1, 2, 3]})";
  ondemand::parser parser;
  ondemand::document doc = parser.iterate(json_str);

  MyStruct my_struct(doc);
  simdjson::json_builder::StringBuilder sb;
  simdjson::json_builder::fast_to_json_string(sb, my_struct);
  std::cout << sb.c_str() << std::endl;

  std::string json_str_nested =
      R"({"a":1,"b":10,"c":0,"d":"test string\n\r\"","e":[1,2,3],"f":["ab","cd","fg"],"y":{"g":100,"h":"test string\n\r\"","i":[1,2,3]}})";
  doc = parser.iterate(json_str_nested);

  X s1 = X(doc);
  simdjson::json_builder::StringBuilder sb2;
  simdjson::json_builder::fast_to_json_string(sb2, s1);
  std::cout << sb2.c_str() << std::endl;

  return 0;
}