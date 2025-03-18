#ifndef SIMDJSON_SERIALIZATION_STRING_BUILDER_HPP
#define SIMDJSON_SERIALIZATION_STRING_BUILDER_HPP
#include "simdjson/json_builder/json_escaping.h"
#include <cstring>
#include <format>
#include <string>
#include <string_view>
#include <vector>
#include <memory>

namespace simdjson {

namespace json_builder {
template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

class string_builder final {
public:
  string_builder(size_t initial_capacity = 1024)
      : buffer(new char[initial_capacity]), position(0),
        capacity(initial_capacity) {}

  template <arithmetic number_type> inline void append(number_type v) {
    if constexpr (std::is_same_v<number_type, bool>) {
      if (v) {
        constexpr char true_literal[] = "true";
        constexpr size_t true_len = sizeof(true_literal) - 1;
        capacity_check(true_len);
        std::memcpy(buffer.get() + position, true_literal, true_len);
        position += true_len;
      } else {
        constexpr char false_literal[] = "false";
        constexpr size_t false_len = sizeof(false_literal) - 1;
        capacity_check(false_len);
        std::memcpy(buffer.get() + position, false_literal, false_len);
        position += false_len;
      }
    } else {
      auto result =
          std::to_chars(buffer.get() + position, buffer.get() + capacity, v);
      if (result.ec != std::errc()) {
        constexpr size_t max_number_size = 20;
        capacity_check(max_number_size);
        result =
            std::to_chars(buffer.get() + position, buffer.get() + capacity, v);
      }
      position = result.ptr - buffer.get();
    }
  }

  inline void append(char c) {
    capacity_check(1);
    buffer[position++] = c;
  }

  void reset() { position = 0; }

  inline void append_escaped_json(std::string_view input) {
    capacity_check(6 * input.size());
    position += simdjson::json_builder::json_escaping::write_string_escaped(
        input, buffer.get() + position);
  }

  inline void append_quoted_escaped_json(std::string_view input) {
    capacity_check(2 + 6 * input.size());
    buffer[position++] = '"';
    position += simdjson::json_builder::json_escaping::write_string_escaped(
        input, buffer.get() + position);
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
    capacity_check(len);
    std::memcpy(buffer.get() + position, str, len);
    position += len;
  }

  operator std::string() const { return std::string(view()); }

  operator std::string_view() const { return view(); }

  std::string_view view() const {
    return std::string_view(buffer.get(), position);
  }

  const char *c_str() {
    capacity_check(1);
    buffer[position] = '\0'; // Ensure null-termination
    return buffer.get();
  }

  size_t size() const { return position; }

  void clear() { position = 0; }

private:
  inline void capacity_check(size_t upcoming_writes) {
    if (position + upcoming_writes > capacity) {
      // We use a simple doubling algorithm.
      size_t new_capacity = capacity * 2;
      if (new_capacity < capacity) {
        throw std::runtime_error("overflow");
      }
      char *new_buffer = new char[new_capacity];
      std::memcpy(new_buffer, buffer.get(), position);
      buffer.reset(new_buffer);
      capacity = new_capacity;
    }
  }
  std::unique_ptr<char[]> buffer;
  size_t position;
  size_t capacity;
};

} // namespace json_builder

} // namespace simdjson
#endif // SIMDJSON_SERIALIZATION_STRING_BUILDER_HPP