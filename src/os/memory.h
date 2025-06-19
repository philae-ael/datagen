// IWYU pragma: private, include "os/os.h"

#include "core/core.h"

void *os_reserve(usize size);
void os_commit(void *ptr, usize size);
void os_decommit(void *ptr, usize size);
void os_release(void *ptr, usize size);

usize os_page_size();
