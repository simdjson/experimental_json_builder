#include <array>
#include <experimental/meta>
#include <iostream>
#include <ostream>
#include <ranges>
#include <utility>
#include <vector>


//////////////////////////////////////////////////////////////////////////////////////////////
// Methods used for the expansion workaround discussed at:
// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2996r1.html#implementation-status
//////////////////////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////////////////////
// End of methods for the expansion workaround. This won't be needed once the expansion
// statements from the C++ 26 reflection paper are implemented. For more details see:
// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2996r1.html#implementation-status
//////////////////////////////////////////////////////////////////////////////////////////////

struct field_descriptor {
  std::string_view field_name;
};

std::ostream &operator<<(std::ostream &os, const field_descriptor &fd) {
  return os << fd.field_name;
}

// Print a single element
template <typename T> void printElement(const T &t) { std::cout << t; }

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

struct X {
  char a;
  int b;
  int c;
  std::string d;
  std::vector<int> e;
  std::vector<std::string> f;
};

template <typename T> constexpr auto struct_to_tuple(T const &t) {
  return [:expand_all(std::meta::nonstatic_data_members_of(^T)):] >> [&]<auto... members> {
    return std::make_tuple(t.[:members:]...);
  };
}

constexpr std::string escape_json(std::string_view input) {
  std::string output;
  output.reserve(input.length());

  for (auto i = 0; i < input.length(); ++i) {
    switch (input[i]) {
    case '"':
      output += "\\\"";
      break;
    case '\b':
      output += "\\b";
      break;
    case '\f':
      output += "\\f";
      break;
    case '\n':
      output += "\\n";
      break;
    case '\r':
      output += "\\r";
      break;
    case '\t':
      output += "\\t";
      break;
    case '\\':
      output += "\\\\";
      break;
    default:
      output += input[i];
      break;
    }
  }

  return output;
}

template <class T>
  requires(std::is_arithmetic_v<T>)
constexpr std::string atom(T t) {
  // not ideal for floats
  return std::to_string(t);
}

template <class T>
  requires(std::is_same_v<T, std::string>)
constexpr std::string atom(T t) {
  return "\"" + escape_json(t) + "\"";
}

template <class T>
  requires(std::is_same_v<T, std::string_view>)
constexpr std::string atom(T t) {
  return "\"" + escape_json(std::string(t)) + "\"";
}

template <class T>
  requires(std::is_same_v<T, const char *>)
constexpr std::string atom(T t) {
  return "\"" + escape_json(std::string(t)) + "\"";
}

// Concept that checks if a type is a container but not a string (because string
// handling is required to be different)
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

template <typename array, class T, size_t... i>
std::string to_json_string(array &desc, const T &t, std::index_sequence<i...>) {
  std::string s = "{";
  std::string quote = "\"";
  (..., (s += (i == 0 ? "" : ", ") + quote + std::string(desc[i].field_name) +
              quote + ":" + atom(std::get<i>(t))));
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

int main() {
  X s1 = {.a = '1',
          .b = 10,
          .c = 0,
          .d = "test string\n\r\"",
          .e = {1, 2, 3},
          .f = {"ab", "cd", "fg"}};

  std::cout << to_json_string(s1) << std::endl;
  return 0;
}