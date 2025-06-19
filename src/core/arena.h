// IWYU pragma: private, include "core/core.h"

#ifndef CORE_ARENA_H
#define CORE_ARENA_H

#include "base.h"
#include <cstring>

struct Arena {
  usize commit_size;
  usize reserve_size;
  usize base;
  usize pos;
  usize commited;
  usize reserved;
};

struct ArenaCreationInfo {
  usize commit_size = KB(64);
  usize reserve_size = MB(64);
};

Arena *arena_alloc(ArenaCreationInfo *infos);
void arena_release(Arena *arena);

void *arena_push(Arena *arena, usize size, u64 align);
usize arena_pos(Arena *arena);
usize arena_pop_to(Arena *arena, usize pos);
inline void arena_clear(Arena *arena) { arena_pop_to(arena, arena->base); }

struct ScopedArena {
  Arena *arena;
  usize pos;

  operator Arena *() { return arena; }
};

ScopedArena arena_scope_enter(Arena *arena);
void arena_scope_exit(ScopedArena);

template <class T> T *arena_push(Arena *arena, usize count = 1) {
  return (T *)std::memset(arena_push(arena, count * sizeof(T), alignof(T)), 0,
                          count * sizeof(T));
}

#endif
