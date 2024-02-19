#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "defines.h"

void *salloc(u64 size);
void sfree(void *p);

#endif /* ALLOCATOR_H */
