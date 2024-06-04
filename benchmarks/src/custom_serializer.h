#include "../../src/json_escaping.hpp"
// For reference, we create a custom serializer.
namespace custom {

constexpr static std::string_view control_chars[] = {
    "\\x0000", "\\x0001", "\\x0002", "\\x0003", "\\x0004", "\\x0005", "\\x0006",
    "\\x0007", "\\x0008", "\\t",     "\\n",     "\\x000b", "\\f",     "\\r",
    "\\x000e", "\\x000f", "\\x0010", "\\x0011", "\\x0012", "\\x0013", "\\x0014",
    "\\x0015", "\\x0016", "\\x0017", "\\x0018", "\\x0019", "\\x001a", "\\x001b",
    "\\x001c", "\\x001d", "\\x001e", "\\x001f"};

// append to output the escaped version of c. The character c must be
// quote, backslash or an ASCII control character.
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
size_t write_quoted_string_direct(const char *name, char *out) {
  const char *const initout = out;
  *out++ = '"';
  out = stpcpy(out, name);
  *out++ = '"';
  return out - initout;
}

size_t write_quoted_string_escaped(std::string_view input, char *out) {
  const char *const initout = out;
  *out++ = '"';
  size_t location = simdjson::experimental::json_escaping::find_next_json_quotable_character(input, 0);
  if (location == input.size()) {
    // no escaping (fast path)
    memcpy(out, input.data(), input.size());
    out += input.size();
  } else {
    memcpy(out, input.data(), location);
    out += location;
    input.remove_prefix(location);
    escape_json_char(input[0], out);
    input.remove_prefix(1);
    // could be optimized in various ways
    while (!input.empty()) {
      location = simdjson::experimental::json_escaping::find_next_json_quotable_character(input, 0);
      memcpy(out, input.data(), location);
      out += location;
      input.remove_prefix(location);
      escape_json_char(input[0], out);
      input.remove_prefix(1);
    }
    *out++ = '"';
  }
  return out - initout;
}

size_t serialize(const Location &u, char *out) {
  const char *const initout = out;
  *out++ = '{';
  out += write_quoted_string_direct("lat", out);
  *out++ = ':';
  auto [ptr1, ec1] = std::to_chars(out, out + 100000, u.lat);
  out = ptr1;
  *out++ = ',';
  out += write_quoted_string_direct("lng", out);
  *out++ = ':';
  auto [ptr2, ec2] = std::to_chars(out, out + 100000, u.lng);
  out = ptr2;
  *out++ = '}';
  return out - initout;
}

size_t serialize(const Profile &u, char *out) {
  // this not meant to be good code.
  const char *const initout = out;
  *out++ = '{';
  out += write_quoted_string_direct("name", out);
  *out++ = ':';
  out += write_quoted_string_escaped(u.name, out);
  *out++ = ',';
  out += write_quoted_string_direct("company", out);
  *out++ = ':';
  out += write_quoted_string_escaped(u.company, out);
  *out++ = ',';
  out += write_quoted_string_direct("dob", out);
  *out++ = ':';
  out += write_quoted_string_escaped(u.dob, out);
  *out++ = ',';
  out += write_quoted_string_direct("address", out);
  *out++ = ':';
  out += write_quoted_string_escaped(u.address, out);
  *out++ = ',';
  out += write_quoted_string_direct("location", out);
  *out++ = ':';
  out += serialize(u.location, out);
  *out++ = ',';
  out += write_quoted_string_direct("about", out);
  *out++ = ':';
  out += write_quoted_string_escaped(u.about, out);
  *out++ = '}';
  return out - initout;
}
size_t serialize(const User &u, char *out) {
  // this not meant to be good code.
  const char *const initout = out;
  *out++ = '{';
  out += write_quoted_string_direct("id", out);
  *out++ = ':';
  out += write_quoted_string_escaped(u.id, out);
  *out++ = ',';
  out += write_quoted_string_direct("email", out);
  *out++ = ':';
  out += write_quoted_string_escaped(u.email, out);
  *out++ = ',';
  out += write_quoted_string_direct("username", out);
  *out++ = ':';
  out += write_quoted_string_escaped(u.username, out);
  *out++ = ',';
  out += write_quoted_string_direct("profile", out);
  *out++ = ':';
  out += serialize(u.profile, out);
  *out++ = ',';
  out += write_quoted_string_direct("apiKey", out);
  *out++ = ':';
  out += write_quoted_string_escaped(u.apiKey, out);
  *out++ = ',';
  out += write_quoted_string_direct("roles", out);
  *out++ = ':';
  *out++ = ',';
  out += write_quoted_string_direct("createdAt", out);
  *out++ = ':';
  out += write_quoted_string_escaped(u.createdAt, out);
  *out++ = ',';
  out += write_quoted_string_direct("updatedAt", out);
  *out++ = ':';
  out += write_quoted_string_escaped(u.updatedAt, out);
  *out++ = '}';
  return out - initout;
}

} // namespace custom
