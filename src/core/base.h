// IWYU pragma: private, include "core/core.h"

#ifndef CORE_BASE_H
#define CORE_BASE_H

// pfff... how to remove them??
// stdexcept should be doable but the rest???
#include <cstdint>
#include <type_traits>

#include "macro.h"

// because there are no ZST types in C++...
// In place of that, use the [[no_unique_address]]...
struct Unit {};
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using usize = std::size_t;

using f32 = float;
static_assert(sizeof(f32) == 4);
using f64 = double;
static_assert(sizeof(f64) == 8);

template <typename T, typename U>
ALWAYS_INLINE constexpr T checked_conversion(U value) {
  auto narrowed = static_cast<T>(value);
  if (static_cast<U>(narrowed) != value) {
    throw "Narrowing conversion failed";
  }
  return narrowed;
}

PRINTF_LIKE(1, 2)
NORETURN void terminate(const char *fmt, ...);
#endif
