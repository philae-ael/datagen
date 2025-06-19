// IWYU pragma: private, include "core/core.h"

#ifndef CORE_MACRO_H
#define CORE_MACRO_H

#define NORETURN [[noreturn]]
#define ALWAYS_INLINE inline __attribute__((always_inline))
#define NEVER_INLINE inline __attribute__((noinline))

#define KB(x) ((x) * 1024)
#define MB(x) ((x) * 1024 * 1024)

#define UNUSED(x) ((void)(x))

#define ALIGN_MASK_UP(x, mask) (((x) + (mask)) & (~(mask)))
#define ALIGN_UP(x, AMOUNT) ((decltype(x))ALIGN_MASK_UP((usize)(x), AMOUNT - 1))

template <class _Fx> struct __defer_t {
  _Fx __fx;

  __defer_t(_Fx &&__arg_fx) : __fx(__arg_fx) {}

  ~__defer_t() { __fx(); }
};

#define TOK_CONCAT(X, Y) X##Y
#define TOK_PASTE(X, Y) TOK_CONCAT(X, Y)
#define defer __defer_t TOK_PASTE(scoped_defer_obj, __COUNTER__) = [&]()

#define __STRINGIFY_(x) #x
#define __STRINGIFY(x) __STRINGIFY_(x)

#define PRINTF_LIKE(fmtidx, argidx) [[gnu::format(printf, fmtidx, argidx)]]

#define CHECK(condition, fmt, ...)                                             \
  do {                                                                         \
    if (!(condition)) {                                                        \
      terminate(fmt __VA_OPT__(, ) __VA_ARGS__);                               \
    }                                                                          \
  } while (0)
#endif
