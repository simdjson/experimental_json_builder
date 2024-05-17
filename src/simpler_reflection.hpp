#include <experimental/meta>
#include <format>
#include <print>

namespace experimental_json_builder {

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

} // namespace experimental_json_builder