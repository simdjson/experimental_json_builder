#include <experimental/meta>
#include "reflection_utils.hpp"
#include <format>
#include <print>

namespace __impl {
  template<auto... vals>
  struct replicator_type {
    template<typename F>
      constexpr void operator>>(F body) const {
        (body.template operator()<vals>(), ...);
      }
  };

  template<auto... vals>
  replicator_type<vals...> replicator = {};
}

template<typename R>
consteval auto expand(R range) {
  std::vector<std::meta::info> args;
  for (auto r : range) {
    args.push_back(reflect_value(r));
  }
  return substitute(^__impl::replicator, args);
}

struct universal_formatter {
  constexpr auto parse(auto& ctx) { return ctx.begin(); }

  template <typename T>
  auto format(T const& t, auto& ctx) const {
    auto out = std::format_to(ctx.out(), "{{");
    auto delim = [first=true, &out]() mutable {
      if (!first) {
        *out++ = ',';
        *out++ = ' ';
      }
      first = false;
    };

    [: expand(bases_of(^T)) :] >> [&]<auto base>{
        delim();
        out = std::format_to(out, "{}", (typename [: type_of(base) :] const&)(t));
    };

    [: expand(nonstatic_data_members_of(^T)) :] >> [&]<auto mem>{
      delim();
      out = std::format_to(out, "\"{}\":{}", name_of(mem), experimental_json_builder::atom(t.[:mem:]));
    };

    *out++ = '}';
    return out;
  }
};