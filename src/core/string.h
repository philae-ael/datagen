// IWYU pragma: private, include "core/core.h"
#ifndef CORE_STRING_H
#define CORE_STRING_H

#include "arena.h"
#include "base.h"
#include <string_view>

struct str8 {
  u8 *data;
  usize len;

  // This compare pointers! care!
  bool is(str8 other) const { return len == other.len && data == other.data; }

  bool equal(str8 other) const {
    return len == other.len && std::memcmp(data, other.data, len) == 0;
  }
};

inline str8 operator""_u8(const char *str, usize len) {
  return str8{(u8 *)str, len};
}

struct str8_list {
  str8_list *next;
  str8 data;
};

template <usize N> ALWAYS_INLINE str8 str8_lit(const char (&lit)[N]) {
  return str8{(u8 *)(lit), N - 1};
}

ALWAYS_INLINE str8 str8_from_cstr(const char *cstr) {
  return str8{(u8 *)cstr, std::strlen(cstr)};
}

struct str8_builder {
  Arena *arena;
  u8 *base;
  usize size;
  usize capacity;

  str8_builder(Arena *arena)
      : arena(arena), base(arena_push<u8>(arena, 0)), size(0) {}

  void append(str8 str);
  void append(std::string_view str) {
    append(str8{(u8 *)str.data(), str.size()});
  }
  void append(const char *str) { append(str8_from_cstr(str)); }
  PRINTF_LIKE(2, 3) void appendf(const char *fmt, ...);
  PRINTF_LIKE(2, 0) void append(const char *fmt, va_list args);

  str8 build(bool null_terminate = false);
  char *build_cstr() { return (char *)build(true).data; }
};

ALWAYS_INLINE char *str8_to_cstr(Arena *arena, str8 str) {
  str8_builder builder(arena);
  builder.append(str);
  return builder.build_cstr();
}

struct Cut {
  str8 head;
  str8 tail;
};

Cut str8_cut(str8 str, u8 c);

inline str8 str8_span(str8 str, usize start) {
  CHECK(start <= str.len, "Invalid span: start=%zu, len=%zu", start, str.len);
  return {str.data + start, str.len - start};
}
inline str8 str8_span(str8 str, usize start, usize size) {
  CHECK(start <= str.len, "Invalid span: start=%zu, len=%zu", start, str.len);
  CHECK(start + size <= str.len, "Invalid span: start=%zu, size=%zu, len=%zu",
        start, size, str.len);
  return {str.data + start, size};
}

struct parsed_float {
  f32 value;
  bool valid;
};
parsed_float parse_float(str8 str);

#endif
