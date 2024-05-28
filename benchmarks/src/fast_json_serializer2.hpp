#pragma once
#include <charconv>
#include <cstring>
#include <experimental/meta>
#include <string_view>
#include <utility>
#include <vector>

namespace fast_json_serializer2 {

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

namespace builder_helper {

static constexpr std::array<uint8_t, 256> json_quotable_character =
    []() constexpr {
      std::array<uint8_t, 256> result{};
      for (int i = 0; i < 32; i++) {
        result[i] = 1;
      }
      for (int i : {'"', '\\'}) {
        result[i] = 1;
      }
      return result;
    }();

constexpr inline size_t
find_next_json_quotable_character(std::string_view view,
                                  size_t location) noexcept {
  auto const str = view.substr(location);
  for (auto pos = str.begin(); pos != str.end(); ++pos) {
    if (json_quotable_character[(uint8_t)*pos]) {
      return pos - str.begin() + location;
    }
  }
  return size_t(view.size());
}

constexpr inline bool needs_escaping(std::string_view view) {
  uint8_t needs = 0;
  for(uint8_t c : view) {
    needs |= json_quotable_character[c];
  }
  return needs;
}

constexpr static std::string_view control_chars[] = {
    "\\x0000", "\\x0001", "\\x0002", "\\x0003", "\\x0004", "\\x0005", "\\x0006",
    "\\x0007", "\\x0008", "\\t",     "\\n",     "\\x000b", "\\f",     "\\r",
    "\\x000e", "\\x000f", "\\x0010", "\\x0011", "\\x0012", "\\x0013", "\\x0014",
    "\\x0015", "\\x0016", "\\x0017", "\\x0018", "\\x0019", "\\x001a", "\\x001b",
    "\\x001c", "\\x001d", "\\x001e", "\\x001f"};

constexpr void escape_json_char(char c, char *&out) {
  if (c == '"') {
    memcpy(out, "\\\"", 2);
    out += 2;
  } else if (c == '\\') {
    memcpy(out, "\\\\", 2);
    out += 2;
  } else {
    std::string_view v = control_chars[uint8_t(c)];
    memcpy(out, v.data(), v.size());
    out += v.size();
  }
}

constexpr inline size_t write_string_escaped(std::string_view input, char *out) {
  const char *const initout = out;
  const char *start = input.data();
  const char *end = start + input.size();
  const char *next_quotable = start;

  while (next_quotable != end) {
      // Find next character that needs escaping
    next_quotable = std::find_if(next_quotable, end, [](char c) {
      return json_quotable_character[static_cast<uint8_t>(c)];
    });

    // Copy characters up to the next quotable character
    if (next_quotable != start) {
      size_t chunk_size = next_quotable - start;
      std::memcpy(out, start, chunk_size);
      out += chunk_size;
      start = next_quotable;
    }

    if (next_quotable != end) {
      // Escape the quotable character
      escape_json_char(*next_quotable, out);
      start++;
      next_quotable++;
    }
  }

  return out - initout;
}

// unoptimized, meant for compile-time execution
consteval std::string to_quoted_escaped(std::string_view input) {
  std::string out = "\"";
  for (char c : input) {
    if (json_quotable_character[uint8_t(c)]) {
      if (c == '"') {
        out.append("\\\"");
      } else if (c == '\\') {
        out.append("\\\\");
      } else {
        std::string_view v = control_chars[uint8_t(c)];
        out.append(v);
      }
    } else {
      out.push_back(c);
    }
  }
  out.push_back('"');
  return out;
}

} // namespace builder_helper

class StringBuilder {
public:
  StringBuilder(size_t initial_capacity = 2629189250)
      : buffer(new char[initial_capacity]), position(0),
        capacity(initial_capacity) {}
  inline void append(double v) {
    auto [ptr, ec] =
        std::to_chars(buffer.get() + position, buffer.get() + capacity, v);
    (void)ec; // no error handling
    position = ptr - buffer.get();
  }

  inline void append(char c) { buffer[position++] = c; }

  void reset() { position = 0; }

  inline void append_escaped_json(std::string_view input) {
    position +=
        builder_helper::write_string_escaped(input, buffer.get() + position);
  }

  inline void append_quoted_escaped_json(std::string_view input) {
    buffer[position++] = '"';
    position +=
        builder_helper::write_string_escaped(input, buffer.get() + position);
    buffer[position++] = '"';
  }

  inline void append(std::string_view v) { append_quoted_escaped_json(v); }

  inline void append_unescaped(const char *c) {
    append_unescaped(std::string_view(c));
  }
  
  inline void append_unescaped(std::string_view str) {
    append_unescaped(str.data(), str.size());
  }

  inline void append_unescaped(const char *str, size_t len) {
    std::memcpy(buffer.get() + position, str, len);
    position += len;
  }

  std::string_view view() const {
    return std::string_view(buffer.get(), position);
  }

  const char *c_str() {
    buffer[position] = '\0'; // Ensure null-termination
    return buffer.get();
  }

  size_t size() const { return position; }

private:
  std::unique_ptr<char[]> buffer;
  size_t position;
  size_t capacity;
};

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

struct field_descriptor {
  std::string field_name;
};

template <typename S> consteval auto print_struct() {
  constexpr size_t N = []() consteval {
    return std::meta::nonstatic_data_members_of(^S).size();
  }();
  std::array<field_descriptor, N> member_size;

  [:expand(std::meta::nonstatic_data_members_of(^S)
           ):] >> [&, i = 0]<auto e>() mutable {
    member_size[i] = {
        .field_name = builder_helper::to_quoted_escaped(std::meta::name_of(e))};
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

template <class T>
  requires(ContainerButNotString<T>)
constexpr void atom(StringBuilder &b, const T& t) {
  b.append('[');
  for (size_t i = 0; i < t.size(); ++i) {
    atom(b, t[i]);
    if (i < t.size() - 1) {
      b.append(',');
    }
  }
  b.append(']');
}

template <class T>
  requires(std::is_same_v<T, std::string> ||
           std::is_same_v<T, std::string_view> ||
           std::is_same_v<T, const char *>)
constexpr void atom(StringBuilder &b, const T &t) {
  b.append(t);
}

template <class T>
  requires(std::is_same_v<T, double>)
constexpr void atom(StringBuilder &b, const T t) {
  b.append(t);
}

template <class T>
  requires(UserDefinedType<T> && !ContainerButNotString<T> &&
           !std::is_same_v<T, std::string> &&
           !std::is_same_v<T, std::string_view> &&
           !std::is_same_v<T, const char *>)
constexpr void atom(StringBuilder &b, const T &t) {
  constexpr auto names = print_struct<T>();
  auto x = struct_to_tuple<T>(t);
  return fast_to_json_string(b, names, x);
}

template <bool isnotzero, class S, class T>
inline void append_key_value(StringBuilder &b, S &&fieldname, T &&value) {
  if (isnotzero) {
    b.append(',');
  }
  b.append_unescaped(fieldname);
  b.append(':');
  atom(b, value);
}

template <typename array, class T, size_t... i>
void fast_to_json_string(StringBuilder &b, array &desc, const T &t,
                         std::index_sequence<i...>) {
  b.append('{');
  (..., (append_key_value<i != 0>(b, desc[i].field_name, std::get<i>(t))));
  b.append('}');
}

template <typename array, class... T>
void fast_to_json_string(StringBuilder &b, array &desc,
                         const std::tuple<T...> &t) {
  fast_to_json_string(b, desc, t, std::make_index_sequence<sizeof...(T)>());
}

// works for struct
template <class Z> void fast_to_json_string(StringBuilder &b, const Z &z) {
  constexpr static auto names = print_struct<Z>();
  auto x = struct_to_tuple<Z>(z);
  fast_to_json_string(b, names, x);
}

// works for container
template <class Z>
  requires(ContainerButNotString<Z>)
void fast_to_json_string(StringBuilder &b, const Z &z) {
  b.append('[');
  for (size_t i = 0; i < z.size(); ++i) {
    atom(b, z[i]);
    if (i < z.size() - 1) {
      b.append(',');
    }
  }
  b.append(']');
}

} // namespace fast_json_serializer2