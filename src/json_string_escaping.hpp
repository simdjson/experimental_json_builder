/**
 * Contains utilities to escape strings for JSON purposes.
 */
#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include "string_builder.hpp"

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

// Find the next character that must be escaped : quote,
// backslash or an ASCII control character.
// This function can be vectorized!!!
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

constexpr static std::string_view control_chars[] = {
    "\\x0000", "\\x0001", "\\x0002", "\\x0003", "\\x0004", "\\x0005", "\\x0006",
    "\\x0007", "\\x0008", "\\t", "\\n", "\\x000b", "\\f", "\\r",
    "\\x000e", "\\x000f", "\\x0010", "\\x0011", "\\x0012", "\\x0013", "\\x0014",
    "\\x0015", "\\x0016", "\\x0017", "\\x0018", "\\x0019", "\\x001a", "\\x001b",
    "\\x001c", "\\x001d", "\\x001e", "\\x001f"};

// append to output the escaped version of c. The character c must be
// quote, backslash or an ASCII control character.
constexpr void escape_json_char(char c, std::string &out) {
  if (c == '"') {
    out.append("\\\"");
  } else if (c == '\\') {
    out.append("\\\\");
  } else {
    out.append(control_chars[uint8_t(c)]);
  }
}

// Append to output the escaped version of c using StringBuilder.
constexpr void escape_json_char(char c, experimental_json_builder::StringBuilder &sb) {
  if (c == '"') {
    sb.append("\\\"");
  } else if (c == '\\') {
    sb.append("\\\\");
  } else {
    sb.append(control_chars[uint8_t(c)]);
  }
}

constexpr void append_quoted_and_escaped_json(std::string_view input,
                                              std::string &out) {
  out.append("\"");
  size_t location = find_next_json_quotable_character(input, 0);
  if (location == input.size()) {
    // no escaping (fast path)
    out.append(input);
  } else {
    out.append(input.substr(0, location));
    input.remove_prefix(location);
    escape_json_char(input[0], out);
    input.remove_prefix(1);
    // could be optimized in various ways
    while (!input.empty()) {
      location = find_next_json_quotable_character(input, 0);
      out.append(input.substr(0, location));
      input.remove_prefix(location);
      escape_json_char(input[0], out);
      input.remove_prefix(1);
    }
  }
  out.append("\"");
}

constexpr void append_quoted_and_escaped_json(std::string_view input,
                                              experimental_json_builder::StringBuilder &sb) {
  sb.append('"');
  size_t location = find_next_json_quotable_character(input, 0);
  if (location == input.size()) {
    // no escaping (fast path)
    sb.append(input);
  } else {
    sb.append(input.substr(0, location));
    input.remove_prefix(location);
    escape_json_char(input[0], sb);
    input.remove_prefix(1);
    // could be optimized in various ways
    while (!input.empty()) {
      location = find_next_json_quotable_character(input, 0);
      sb.append(input.substr(0, location));
      input.remove_prefix(location);
      escape_json_char(input[0], sb);
      input.remove_prefix(1);
    }
  }
  sb.append('"');
}