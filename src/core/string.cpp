#include "core.h"
#include <cstdarg>
#include <cstdio>

void str8_builder::append(str8 str) {
  auto *data = arena_push<u8>(arena, str.len);
  std::memcpy(data, str.data, str.len);
  size += str.len;
}

void str8_builder::appendf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  append(fmt, args);
  va_end(args);
}

void str8_builder::append(const char *fmt, va_list args) {
  va_list args_copy;
  va_copy(args_copy, args);
  auto len_ = std::vsnprintf(nullptr, 0, fmt, args_copy);
  va_end(args_copy);
  if (len_ < 0) {
    terminate("Failed to format string: %s", fmt);
  }
  usize len = (usize)len_;

  auto *data = arena_push<u8>(arena, len + 1);
  std::vsnprintf((char *)data, len + 1, fmt, args);
  arena_pop_to(arena, arena->pos - 1);

  size += len;
}

str8 str8_builder::build(bool null_terminate) {
  if (null_terminate) {
    append("\0"_u8);
  }
  return {base, size};
}

Cut str8_cut(str8 str, u8 c) {
  usize i;
  for (i = 0; i < str.len && str.data[i] != c; ++i)
    ;
  return {str8_span(str, 0, i), str8_span(str, i)};
}

parsed_float parse_float(str8 s) {
  float r = 0.0f;
  float sign = 1.0f;
  float exp = 0.0f;
  for (auto i = 0uz; i < s.len; i++) {
    switch (s.data[i]) {
    case '+':
      break;
    case '-':
      sign = -1;
      break;
    case '.':
      exp = 1;
      break;
    case 'E':
    case 'e':
      // Not implemented: scientific notation
      return {0.0f, false};
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      r = 10.0f * r + (s.data[i] - '0');
      exp *= 0.1f;
      break;
    default:
      return {0.0f, false};
    }
  }
  return {sign * r * (exp ? exp : 1.0f), true};
}
