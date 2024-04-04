// reflection_utils.hpp
#pragma once

#include "meta_utils.hpp"
#include <array>
#include <ostream>
#include <string_view>

namespace experimental_json_builder {

struct field_descriptor {
  std::string_view field_name;
};

std::ostream &operator<<(std::ostream &os, const field_descriptor &fd) {
  return os << fd.field_name;
}

template <typename S> consteval auto print_struct() {
  constexpr size_t N = []() consteval {
    return std::meta::nonstatic_data_members_of(^S).size();
  }();
  std::array<field_descriptor, N> member_size;

  [:expand(std::meta::nonstatic_data_members_of(^S)
           ):] >> [&, i = 0]<auto e>() mutable {
    member_size[i] = {.field_name = std::meta::name_of(e)};
    ++i;
  };
  return member_size;
};

template <typename T> constexpr auto struct_to_tuple(T const &t) {
  return [:expand_all(std::meta::nonstatic_data_members_of(^T)
                      ):] >> [&]<auto... members> {
    return std::make_tuple(t.[:members:]...);
  };
}

} // namespace experimental_json_builder
