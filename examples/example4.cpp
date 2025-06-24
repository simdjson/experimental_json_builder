#include "simdjson/json_builder/json_builder.h"
#include "simdjson/json_builder/string_builder.h"
#include <cstdlib>
#include <format>
#include <iostream>
#include <print>
#include <string>
#include <vector>
#include <tuple>
struct Z {
  int x;
};

struct Y {
  int g;
  std::string h;
  std::vector<int> i;
  Z z;
};

struct X {
  char a;
  int b;
  int c;
  std::string d;
  std::vector<int> e;
  std::vector<std::string> f;
  Y y;
};


// A simple method name/value data structure
 template <typename T>
 struct to {
   T *pointer;
   std::string_view m_key;
   constexpr to(std::string_view const inp_key) noexcept
       : pointer{}, m_key{} {}
   constexpr to(std::string_view const inp_key, T &obj_ref) noexcept
       : pointer{std::addressof(obj_ref)}, m_key{inp_key} {}
 };


// just a bogus concept
template<typename T>
concept Keyed = requires(T t) {
    { t.m_key } -> std::same_as<std::string_view&>;
    { *t.pointer };
};

// extract takes instances of the form 'to'.
template <Keyed ...Funcs>
bool extract(Funcs&&... endpoints) {
    ((std::cout << endpoints.m_key << '\n'), ...);
    return true;
}

/////////////////////
// Begin struct_to_tuple
namespace __impl {
  template<auto... vals>
  struct replicator_type {
    template<typename F>
      constexpr auto operator>>(F body) const -> decltype(auto) {
        return body.template operator()<vals...>();
      }
  };

  template<auto... vals>
  replicator_type<vals...> replicator = {};
}

template<typename R>
consteval auto expand_all(R range) {
  std::vector<std::meta::info> args;
  for (auto r : range) {
    args.push_back(reflect_value(r));
  }
  return substitute(^__impl::replicator, args);
}

template <typename T>
constexpr auto struct_to_tuple(T const& t) {
  return [: expand_all(nonstatic_data_members_of(^T)) :] >> [&]<auto... members>{
    return std::make_tuple(to(std::meta::identifier_of(members), t.[:members:])...);
  };
}
// End struct_to_tuple
////////////////////////

int main() {
  X x;
  std::apply([](auto&&... xs){ extract(xs...);}, struct_to_tuple(x));
  return EXIT_SUCCESS;
}