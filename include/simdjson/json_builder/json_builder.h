#ifndef SIMDJSON_SERIALIZATION_JSON_BUILDER_HPP
#define SIMDJSON_SERIALIZATION_JSON_BUILDER_HPP

#include "json_escaping.h"
#include "string_builder.h"
#include <charconv>
#include <cstring>
#include <experimental/meta>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace simdjson {

namespace json_builder {

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

template <typename T>
concept UserDefinedType = std::is_class<T>::value;

// workaround from
// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2996r3.html#back-and-forth
// for missing expansion statements
namespace __impl {
template <auto... vals> struct replicator_type {
  template <typename F> constexpr void operator>>(F body) const {
    (body.template operator()<vals>(), ...);
  }
};

template <auto... vals> replicator_type<vals...> replicator = {};
} // namespace __impl

template <typename R> consteval auto expand(R range) {
  std::vector<std::meta::info> args;
  for (auto r : range) {
    args.push_back(std::meta::reflect_value(r));
  }
  return substitute(^__impl::replicator, args);
}
// end of workaround

template <class T>
  requires(ContainerButNotString<T>)
constexpr void atom(string_builder &b, const T &t) {
  if (t.size() == 0) {
    b.append_unescaped("[]");
    return;
  }
  b.append('[');
  atom(b, t[0]);
  for (size_t i = 1; i < t.size(); ++i) {
    b.append(',');
    atom(b, t[i]);
  }
  b.append(']');
}

template <class T>
  requires(std::is_same_v<T, std::string> ||
           std::is_same_v<T, std::string_view> ||
           std::is_same_v<T, const char *>)
constexpr void atom(string_builder &b, const T &t) {
  b.append(t);
}

template <arithmetic T> constexpr void atom(string_builder &b, const T t) {
  b.append(t);
}

template <class T>
  requires(UserDefinedType<T> && !ContainerButNotString<T> &&
           !std::is_same_v<T, std::string> &&
           !std::is_same_v<T, std::string_view> &&
           !std::is_same_v<T, const char *>)
constexpr void atom(string_builder &b, const T &t) {
  int i = 0;
  b.append('{');
  [:expand(std::meta::nonstatic_data_members_of(^T)):] >> [&]<auto dm> {
    if (i != 0)
      b.append(',');
    constexpr auto v =
        simdjson::json_builder::json_escaping::to_quoted_escaped(
            std::meta::identifier_of(dm));
    b.append_unescaped(v);
    b.append(':');
    atom(b, t.[:dm:]);
    i++;
  };
  b.append('}');
}

// works for struct
template <class Z> void fast_to_json_string(string_builder &b, const Z &z) {
  int i = 0;
  b.append('{');
  [:expand(std::meta::nonstatic_data_members_of(^Z)):] >> [&]<auto dm> {
    if (i != 0)
      b.append(',');
    constexpr auto v =
        simdjson::json_builder::json_escaping::to_quoted_escaped(
            std::meta::identifier_of(dm));
    b.append_unescaped(v);
    b.append(':');
    atom(b, z.[:dm:]);
    i++;
  };
  b.append('}');
}

// works for container
template <class Z>
  requires(ContainerButNotString<Z>)
void fast_to_json_string(string_builder &b, const Z &z) {
  if (z.size() == 0) {
    b.append_unescaped("[]");
    return;
  }
  b.append('[');
  atom(b, z[0]);
  for (size_t i = 1; i < z.size(); ++i) {
    b.append(',');
    atom(b, z[i]);
  }
  b.append(']');
}

template <class Z>
std::string to_json_string(const Z &z) {
  string_builder b;
  fast_to_json_string(b, z);
  return std::string(b);
}

template <class Z>
void to_json(const Z &z, std::string &s) {
  string_builder b;
  fast_to_json_string(b, z);
  s.assign(b.view());
}

} // namespace json_builder

} // namespace simdjson
#endif