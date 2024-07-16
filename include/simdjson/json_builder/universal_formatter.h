#ifndef SIMDJSON_SERIALIZATION_UNIVERSAL_FORMATTER_HPP
#define SIMDJSON_SERIALIZATION_UNIVERSAL_FORMATTER_HPP
#include "simdjson/json_builder/string_builder.hpp"
#include <experimental/meta>
#include <format>
#include <print>

namespace simdjson {

namespace json_builder {

struct universal_formatter {
  constexpr auto parse(auto &ctx) { return ctx.begin(); }

  template <typename T> auto format(T const &t, StringBuilder &sb) const {
    sb.append('{');
    auto delim = [first = true, &sb]() mutable {
      if (!first) {
        sb.append(',');
        sb.append(' ');
      }
      first = false;
    };

    [:expand(bases_of(^T)):] >> [&]<auto base> {
      delim();
      sb.append(std::format("{}", (typename[:type_of(base):] const &)(t)));
    };

    [:expand(nonstatic_data_members_of(^T)):] >> [&]<auto mem> {
      delim();
      sb.append(std::format("\"{}\":{}", name_of(mem),
                            json_builder::atom(sb, t.[:mem:])));
    };

    sb.append('}');
  }
};

} // namespace json_builder

} // namespace simdjson

#endif