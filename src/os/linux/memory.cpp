#include <sys/mman.h>
#include <unistd.h>

#include "core/core.h"

usize os_page_size() {
#ifdef _SC_PAGESIZE
  return usize(sysconf(_SC_PAGESIZE));
#else
  #error "os_page_size not implemented in this environment"
#endif
}

void* os_reserve(usize size) {
  auto ptr = mmap(nullptr, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  return ptr;
}

void os_commit(void* ptr, usize size) {
  mprotect(ptr, size, PROT_READ | PROT_WRITE);
}

void os_decommit(void* ptr, usize size) {
  madvise(ptr, size, MADV_DONTNEED);
  mprotect(ptr, size, PROT_NONE);
}

void os_release(void* ptr, usize size) {
  munmap(ptr, size);
}
