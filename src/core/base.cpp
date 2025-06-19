#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include "core.h"

PRINTF_LIKE(1, 2) void terminate(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  auto len_ = std::vsnprintf(nullptr, 0, fmt, args);
  va_end(args);

  if (len_ < 0) {
    // Handle error in formatting
    std::fprintf(stderr, "Error formatting string: %s\n", fmt);
    std::abort();
  }
  auto len = usize(len_);

  auto *buffer = new char[len + 1];
  va_start(args, fmt);
  std::vsnprintf(buffer, len + 1, fmt, args);
  va_end(args);

  std::fprintf(stderr, "%s\n", buffer);
  delete[] buffer;

  std::abort();
}
