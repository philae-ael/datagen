#include "core.h"
#include "os/os.h"

Arena *arena_alloc(ArenaCreationInfo *infos) {
  usize commit_size = ALIGN_UP(infos->commit_size, os_page_size());
  usize reserve_size = ALIGN_UP(infos->reserve_size, os_page_size());

  Arena *arena = (Arena *)os_reserve(infos->reserve_size);
  CHECK(arena, "Failed to reserve memory for arena");
  os_commit(arena, commit_size);

  *arena = Arena{
      .commit_size = commit_size,
      .reserve_size = infos->reserve_size,
      .base = sizeof(Arena),
      .pos = sizeof(Arena),
      .commited = commit_size,
      .reserved = reserve_size,
  };

  return arena;
}

void arena_release(Arena *arena) {
  auto reserved = arena->reserved;
  auto commited = arena->commited;

  os_decommit(arena, commited);
  os_release(arena, reserved);
}

void *arena_push(Arena *arena, usize size, u64 align) {
  usize pos = ALIGN_UP(arena->pos, align);
  if (pos + size > arena->commited) {
    CHECK(arena->commited + arena->commit_size <= arena->reserved,
          "Arena out of memory: not enough reserved space");

    os_commit(arena + arena->commited, arena->commit_size);
    arena->commited += arena->commit_size;
  }

  void *ptr = (void *)((u8 *)arena + pos);
  arena->pos = pos + size;

  return ptr;
}

usize arena_pos(Arena *arena) { return arena->pos; }

usize arena_pop_to(Arena *arena, usize pos) {
  CHECK(pos <= arena->pos,
        "Arena pop to position is greater than current position");

  usize old_pos = arena->pos;
  arena->pos = pos;

  return old_pos;
}

ScopedArena arena_scope_enter(Arena *arena) {
  return ScopedArena{.arena = arena, .pos = arena->pos};
}
void arena_scope_exit(ScopedArena scoped) {
  arena_pop_to(scoped.arena, scoped.pos);
}
