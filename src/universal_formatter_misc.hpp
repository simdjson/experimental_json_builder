#include <experimental/meta>
#include "string_builder.hpp"
#include <format>
#include <print>

/*
Leaving this here for now, but it's not clear that we will use/need it.
*/

namespace experimental_json_builder {

struct universal_formatter {
  constexpr auto parse(auto& ctx) { return ctx.begin(); }

  template <typename T>
  auto format(T const& t, StringBuilder& sb) const {
    sb.append('{');
    auto delim = [first=true, &sb]() mutable {
      if (!first) {
        sb.append(',');
        sb.append(' ');
      }
      first = false;
    };

    [: expand(bases_of(^T)) :] >> [&]<auto base>{
        delim();
        sb.append(std::format("{}", (typename [: type_of(base) :] const&)(t)));
    };

    [: expand(nonstatic_data_members_of(^T)) :] >> [&]<auto mem>{
      delim();
      sb.append(std::format("\"{}\":{}", name_of(mem), experimental_json_builder::atom(sb, t.[:mem:])));
    };

    sb.append('}');
  }
};

} // namespace experimental_json_builder