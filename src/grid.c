#include "grid.h"

#include "logger.h"
#include "allocator.h"

#include <string.h>

Grid *createGrid(u64 size) {
    /* if (size % 8 != 0) { */
    /*     ERROR("grid invalid create size\n"); */
    /*     return NULL; */
    /* } */

    Grid *p = slMalloc(sizeof(Grid));
    p->data = slMalloc(sizeof(bool) * size);
    p->size = size;
    memset(p->data, 0, sizeof(bool) * p->size);

    return p;
}

void destroyGrid(Grid *p) {
    slFree(p->data);
    slFree(p);
}

void gridReset(Grid *p) {
    memset(p->data, 0, sizeof(bool) * p->size);
}

bool gridGet(Grid *p, u64 pos) {
    /* u64 i = pos / 8; */
    /* u64 remain = pos % 8; */

    /* char *ptr = (char *)p->data; */
    /* u64 cmp = ptr[i]; */
    /* if (cmp & (1 << remain)) { */
    /*     return true; */
    /* } else { */
    /*     return false; */
    /* } */

    return p->data[pos];
}

void gridSet(Grid *p, u64 pos, bool val) {
    /* u64 i = pos / 8; */
    /* u64 remain = pos % 8; */

    /* u8 *ptr = (u8 *)p->data; */
    /* u8 temp = ptr[i]; */
    /* if (val) { */
    /*     ptr[i] = temp | (u8)(1 << pos); */
    /* } else { */
    /*     ptr[i] = temp & ~(u8)(1 << pos); */
    /* } */

    p->data[pos] = val;
}

u64 gridEmptyArray(Grid *p, u64 *arr) {
    u64 j = 0;
    for (u64 i = 0; i < p->size; i++) {
        if (gridGet(p, i) == false) {
            arr[j] = i;
            j += 1;
        }
    }

    return j;
}

u64 gridArray(Grid *p, u64 *arr) {
    u64 j = 0;
    for (u64 i = 0; i < p->size; i++) {
        if (gridGet(p, i)) {
            arr[j] = i;
            j += 1;
        }
    }

    return j;
}
