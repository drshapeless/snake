#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "defines.h"

void *slAlloc(u64 size);
void *slRealloc(void *ptr, u64 size);
void slFree(void *ptr);

#endif /* ALLOCATOR_H */
