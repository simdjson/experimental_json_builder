// tuple_utils.hpp
#pragma once

#include <iostream>
#include <tuple>

namespace experimental_json_builder {

/*
// Overload << for std::tuple
template <typename... Args>
std::ostream &operator<<(std::ostream &os, const std::tuple<Args...> &tup) {
  os << "(";
  std::apply(
      [&os](const Args &...args) {
        // Using a fold expression to iterate through tuple elements
        ((os << args << (", ")), ...);
      },
      tup);
  return os << ")"; // Erase the last comma and space, then close parenthesis
}*/

} // namespace experimental_json_builder
