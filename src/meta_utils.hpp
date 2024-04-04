// meta_utils.hpp
#pragma once

#include <experimental/meta>
#include <utility>
#include <vector>

namespace experimental_json_builder {

// This leverages the workaround discussed at:
// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2996r1.html#implementation-status
// later on this can be deleted once expand functionality is implemented.

namespace __impl {
template <auto... vals> struct replicator_type_expand_all {
  template <typename F>
  constexpr auto operator>>(F body) const -> decltype(auto) {
    return body.template operator()<vals...>();
  }
};

template <auto... vals> struct replicator_type {
  template <typename F> constexpr void operator>>(F body) const {
    (body.template operator()<vals>(), ...);
  }
};

template <auto... vals>
replicator_type_expand_all<vals...> replicator_expand_all = {};

template <auto... vals> replicator_type<vals...> replicator_expand = {};
} // namespace __impl

template <typename R> consteval auto expand_all(R range) {
  std::vector<std::meta::info> args;
  for (auto r : range) {
    args.push_back(std::meta::reflect_value(r));
  }
  return substitute(^__impl::replicator_expand_all, args);
}

template <typename R> consteval auto expand(R range) {
  std::vector<std::meta::info> args;
  for (auto r : range) {
    args.push_back(std::meta::reflect_value(r));
  }
  return substitute(^__impl::replicator_expand, args);
}

} // namespace experimental_json_builder
