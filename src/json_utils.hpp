// json_utils.hpp
#pragma once

#include "reflection_utils.hpp"
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>

namespace experimental_json_builder {

constexpr std::string escape_json(std::string_view input) {
  std::string output;
  output.reserve(input.length());

  for (auto i = 0; i < input.length(); ++i) {
    switch (input[i]) {
    case '"':
      output += "\\\"";
      break;
    case '\b':
      output += "\\b";
      break;
    case '\f':
      output += "\\f";
      break;
    case '\n':
      output += "\\n";
      break;
    case '\r':
      output += "\\r";
      break;
    case '\t':
      output += "\\t";
      break;
    case '\\':
      output += "\\\\";
      break;
    default:
      output += input[i];
      break;
    }
  }

  return output;
}

template <class T>
  requires(std::is_arithmetic_v<T>)
constexpr std::string atom(T t) {
  // not ideal for floats
  return std::to_string(t);
}

template <class T>
  requires(std::is_same_v<T, std::string>)
constexpr std::string atom(T t) {
  return "\"" + escape_json(t) + "\"";
}

template <class T>
  requires(std::is_same_v<T, std::string_view>)
constexpr std::string atom(T t) {
  return "\"" + escape_json(std::string(t)) + "\"";
}

template <class T>
  requires(std::is_same_v<T, const char *>)
constexpr std::string atom(T t) {
  return "\"" + escape_json(std::string(t)) + "\"";
}

// Concept that checks if a type is a container but not a string (because
// strings handling must be handled differently)
template <typename T>
concept ContainerButNotString =
    requires(T a) {
      { a.size() } -> std::convertible_to<std::size_t>;
      {
        a[std::declval<std::size_t>()]
      }; // check if elements are accessible for the subscript operator
    } && !std::is_same_v<T, std::string> &&
    !std::is_same_v<T, std::string_view> && !std::is_same_v<T, const char *>;

template <class T>
  requires(ContainerButNotString<T>)
constexpr std::string atom(T t) {
  std::string result = "[";
  for (size_t i = 0; i < t.size(); ++i) {
    result += atom(t[i]);
    if (i < t.size() - 1)
      result += ", ";
  }
  result += "]";
  return result;
}

template <typename array, class T, size_t... i>
std::string to_json_string(array &desc, const T &t, std::index_sequence<i...>) {
  std::string s = "{";
  std::string quote = "\"";
  (..., (s += (i == 0 ? "" : ", ") + quote + std::string(desc[i].field_name) +
              quote + ":" + atom(std::get<i>(t))));
  s += "}";
  return s;
}

template <typename array, class... T>
std::string to_json_string(array &desc, const std::tuple<T...> &t) {
  return to_json_string(desc, t, std::make_index_sequence<sizeof...(T)>());
}

template <typename array, class T, size_t... i>
void print_json(array &desc, const T &t, std::index_sequence<i...>) {
  std::cout << "{";
  (...,
   (std::cout << (i == 0 ? "" : ", ") << '"' << escape_json(desc[i].field_name)
              << '"' << ':' << std::get<i>(t)));
  std::cout << "}" << std::endl;
}

template <typename array, class... T>
void print_json(array &desc, const std::tuple<T...> &t) {
  print_json(desc, t, std::make_index_sequence<sizeof...(T)>());
}

template <class Z> std::string to_json_string(Z z) {
  constexpr auto names = print_struct<Z>();
  auto x = struct_to_tuple<Z>(z);
  return to_json_string(names, x);
}

} // namespace experimental_json_builder
