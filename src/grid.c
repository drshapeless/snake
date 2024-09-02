#include "grid.h"

#include "logger.h"
#include "allocator.h"

#include <string.h>

Grid *createGrid(u64 size) {
    if (size % 8 != 0) {
        ERROR("grid invalid create size\n");
        return NULL;
    }

    Grid *p = slMalloc(sizeof(Grid));
    p->data = slMalloc(sizeof(u8) * size / 8);
    p->size = size;

    return p;
}

void destroyGrid(Grid *p) {
    slFree(p->data);
    slFree(p);
}

void gridReset(Grid *p) {
    memset(p->data, 0, p->size / 8);
}

bool gridGet(Grid *p, u64 pos) {
    u32 i = pos / 8;
    u32 remain = pos % 8;

    char *ptr = (char *)p->data;
    u32 cmp = ptr[i];
    if (cmp & (1 << remain)) {
        return true;
    } else {
        return false;
    }
}

void gridSet(Grid *p, u64 pos, bool val) {
    u32 i = pos / 8;
    u32 remain = pos % 8;

    char *ptr = (char *)p->data;
    u32 temp = ptr[i];
    if (val) {
        ptr[i] = temp | (char)(1 << pos);
    } else {
        ptr[i] = temp & ~(char)(1 << pos);
    }
}
