// json_utils.hpp
#pragma once
#include "string_builder.hpp"
#include "reflection_utils.hpp"
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>

namespace experimental_json_builder {


template <class T>
  requires(std::is_arithmetic_v<T>)
constexpr std::string atom(T t) {
  // not ideal for floats
  return std::to_string(t);
}

template <class T>
  requires(std::is_same_v<T, std::string>)
constexpr std::string atom(T t) {
  std::string out;
  append_quoted_and_escaped_json(t, out);
  return out;
}

template <class T>
  requires(std::is_same_v<T, std::string_view>)
constexpr std::string atom(T t) {
  std::string out;
  append_quoted_and_escaped_json(t, out);
  return out;
}

template <class T>
  requires(std::is_same_v<T, const char *>)
constexpr std::string atom(T t) {
  std::string out;
  append_quoted_and_escaped_json(std::string_view(t), out);
  return out;
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

// Code below is needed to make things work for nested structures.
// TODO(FranciscoThiesen): This atom function might lead to issues with STL classes, as it is unclear that they should receive the same treatment as user-defined types.

template<typename T>
concept UserDefinedType = std::is_class<T>::value;

template <class T>
  requires(UserDefinedType<T> && !ContainerButNotString<T> && !std::is_same_v<T, std::string> &&
    !std::is_same_v<T, std::string_view> && !std::is_same_v<T, const char *>)
constexpr std::string atom(T t) {
  constexpr auto names = print_struct<T>();
  auto x = struct_to_tuple<T>(t);
  return to_json_string(names, x);
}

template <size_t i, class S, class T>
inline void append_key_value(S&& fieldname, T&& value, std::string& out) {
  out += (i == 0 ? "" : ", ");
  append_quoted_and_escaped_json(fieldname, out);
  out += ":";
  out += atom(value);
}

template <typename array, class T, size_t... i>
std::string to_json_string(array &desc, const T &t, std::index_sequence<i...>) {
  std::string s = "{";
  (..., (append_key_value<i>(desc[i].field_name, std::get<i>(t), s)));
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

/* Code below is an alternative version that uses stringBuilder 
   This obviously leads to code duplication, but the main idea here
   is to assess whether there are significant performance gains if we
   use a string builder instead of dynamically allocating/appending
   strings.
*/

template <class T>
requires(std::is_arithmetic_v<T>)
constexpr void atom(T t, StringBuilder& sb) {
    sb.append(std::to_string(t));
}

template <class T>
requires(std::is_same_v<T, std::string>)
constexpr void atom(T t, StringBuilder& sb) {
    append_quoted_and_escaped_json(t, sb);
}

template <class T>
requires(std::is_same_v<T, std::string_view>)
constexpr void atom(T t, StringBuilder& sb) {
    append_quoted_and_escaped_json(t, sb);
}

template <class T>
requires(std::is_same_v<T, const char*>)
constexpr void atom(T t, StringBuilder& sb) {
    append_quoted_and_escaped_json(std::string_view(t), sb);
}

template <class T>
requires(ContainerButNotString<T>)
constexpr void atom(T t, StringBuilder& sb) {
    sb.append('[');
    for (size_t i = 0; i < t.size(); ++i) {
        atom(t[i], sb);
        if (i < t.size() - 1) {
            sb.append(", ");
        }
    }
    sb.append(']');
}

template <class T>
requires(UserDefinedType<T> && !ContainerButNotString<T> && !std::is_same_v<T, std::string> &&
    !std::is_same_v<T, std::string_view> && !std::is_same_v<T, const char*>)
constexpr void atom(T t, StringBuilder& sb) {
    constexpr auto names = print_struct<T>();
    auto x = struct_to_tuple<T>(t);
    to_json_string(names, x, sb);
}

template <size_t i, class S, class T>
inline void append_key_value(S&& fieldname, T&& value, StringBuilder& sb) {
    if (i > 0) {
        sb.append(std::string_view(", "));
    }
    append_quoted_and_escaped_json(fieldname, sb);
    sb.append(':');
    atom(value, sb);
}

template <typename array, class T, size_t... i>
void to_json_string(array& desc, const T& t, StringBuilder& sb, std::index_sequence<i...>) {
    sb.append('{');
    (..., (append_key_value<i>(desc[i].field_name, std::get<i>(t), sb)));
    sb.append('}');
}

template <typename array, class... T>
void to_json_string(array& desc, const std::tuple<T...>& t, StringBuilder& sb) {
    to_json_string(desc, t, sb, std::make_index_sequence<sizeof...(T)>());
}

template <class Z>
void to_json_string(Z z, StringBuilder& sb) {
    constexpr auto names = print_struct<Z>();
    auto x = struct_to_tuple<Z>(z);
    to_json_string(names, x, sb);
}


} // namespace experimental_json_builder