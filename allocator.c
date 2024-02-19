#include "allocator.h"

#include <stdlib.h>

void *salloc(u64 size) {
    return malloc(size);
}

void sfree(void *p) {
    free(p);
}
