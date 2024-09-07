#include "allocator.h"

#include <stdlib.h>

void *slAlloc(u64 size) {
    return malloc(size);
}

void *slRealloc(void *ptr, u64 size) {
    return realloc(ptr, size);
}

void slFree(void *ptr) {
    free(ptr);
}
