#include <array>
#include <cstdint>

#if defined(__SSE2__) || defined(__x86_64__) || defined(__x86_64) ||           \
    (defined(_M_AMD64) || defined(_M_X64) ||                                   \
     (defined(_M_IX86_FP) && _M_IX86_FP == 2))
#ifndef SIMDJSON_EXPERIMENTAL_HAS_SSE2
#define SIMDJSON_EXPERIMENTAL_HAS_SSE2 1
#endif
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
#ifndef SIMDJSON_EXPERIMENTAL_HAS_NEON
#define SIMDJSON_EXPERIMENTAL_HAS_NEON 1
#endif
#endif

#if SIMDJSON_EXPERIMENTAL_HAS_NEON
#include <arm_neon.h>
#endif
#if SIMDJSON_EXPERIMENTAL_HAS_SSE2
#include <emmintrin.h>
#endif

namespace simdjson {
namespace experimental {
namespace json_escaping {

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

constexpr inline bool simple_needs_escaping(std::string_view v) {
  for (char c : v) {
    if ((uint8_t(c) < 32) | (c == '"') | (c == '\\')) {
      return true;
    }
  }
  return false;
}

constexpr inline bool branchless_needs_escaping(std::string_view v) {
  bool b = false;
  for (char c : v) {
    b |= ((uint8_t(c) < 32) | (c == '"') | (c == '\\'));
  }
  return b;
}

constexpr inline bool table_needs_escaping(std::string_view view) {
  uint8_t needs = 0;
  for (uint8_t c : view) {
    needs |= json_quotable_character[c];
  }
  return needs;
}

#if SIMDJSON_EXPERIMENTAL_HAS_NEON

inline bool fast_needs_escaping(std::string_view view) {
  if (view.size() < 16) {
    return simple_needs_escaping(view);
  }
  size_t i = 0;
  static uint8_t rnt_array[16] = {1, 0, 34, 0, 0,  0, 0, 0,
                                  0, 0, 0,  0, 92, 0, 0, 0};
  const uint8x16_t rnt = vld1q_u8(rnt_array);
  uint8x16_t running{0};
  for (; i + 15 < view.size(); i += 16) {
    uint8x16_t word = vld1q_u8((const uint8_t *)view.data() + i);
    running = vorrq_u8(running, vceqq_u8(vqtbl1q_u8(rnt, word), word));
    running = vorrq_u8(running, vcltq_u8(word, vdupq_n_u8(32)));
  }
  if (i < view.size()) {
    uint8x16_t word =
        vld1q_u8((const uint8_t *)view.data() + view.length() - 16);
    running = vorrq_u8(running, vceqq_u8(vqtbl1q_u8(rnt, word), word));
    running = vorrq_u8(running, vcltq_u8(word, vdupq_n_u8(32)));
  }
  return vmaxvq_u32(vreinterpretq_u32_u8(running)) != 0;
}

#elif SIMDJSON_EXPERIMENTAL_HAS_SSE2

inline bool fast_needs_escaping(std::string_view view) {
  if (view.size() < 16) {
    return simple_needs_escaping(view);
  }
  size_t i = 0;
  __m128i running = _mm_setzero_si128();
  for (; i + 15 < view.size(); i += 16) {
    __m128i word = _mm_loadu_si128((const __m128i *)(view.data() + i));
    running = _mm_or_si128(running, _mm_cmpeq_epi8(word, _mm_set1_epi8(34)));
    running = _mm_or_si128(running, _mm_cmpeq_epi8(word, _mm_set1_epi8(92)));
    running = _mm_or_si128(
        running, _mm_cmpeq_epi8(_mm_subs_epu8(word, _mm_set1_epi8(31)),
                                _mm_setzero_si128()));
  }
  if (i < view.size()) {
    __m128i word =
        _mm_loadu_si128((const __m128i *)(view.data() + view.length() - 16));
    running = _mm_or_si128(running, _mm_cmpeq_epi8(word, _mm_set1_epi8(34)));
    running = _mm_or_si128(running, _mm_cmpeq_epi8(word, _mm_set1_epi8(92)));
    running = _mm_or_si128(
        running, _mm_cmpeq_epi8(_mm_subs_epu8(word, _mm_set1_epi8(31)),
                                _mm_setzero_si128()));
  }
  return _mm_movemask_epi8(running) != 0;
}

#else

inline bool fast_needs_escaping(std::string_view view) {
  return branchless_needs_escaping(view);
}
#endif

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

constexpr inline size_t write_string_escaped(std::string_view input,
                                             char *out) {
  if (!fast_needs_escaping(input)) { // fast path!
    memcpy(out, input.data(), input.size());
    return input.size();
  }
  const char *const initout = out;
  size_t location = find_next_json_quotable_character(input, 0);
  memcpy(out, input.data(), location);
  out += location;
  input.remove_prefix(location);
  escape_json_char(input[0], out);
  input.remove_prefix(1);
  // could be optimized in various ways
  while (!input.empty()) {
    location = find_next_json_quotable_character(input, 0);
    memcpy(out, input.data(), location);
    out += location;
    input.remove_prefix(location);
    escape_json_char(input[0], out);
    input.remove_prefix(1);
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
} // namespace json_escaping
} // namespace experimental
} // namespace simdjson