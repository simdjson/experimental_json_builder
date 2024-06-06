#pragma once
#include <vector>
#include <string>
#include <string_view>
#include <cstring>
#include <format>

namespace experimental_json_builder {

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
    position += simdjson::experimental::json_escaping::write_string_escaped(
        input, buffer.get() + position);
  }

  inline void append_quoted_escaped_json(std::string_view input) {
    buffer[position++] = '"';
    position += simdjson::experimental::json_escaping::write_string_escaped(
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

  void clear() { position = 0; }

private:
  std::unique_ptr<char[]> buffer;
  size_t position;
  size_t capacity;
};

} // namespace experimental_json_builder