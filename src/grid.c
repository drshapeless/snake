#include "grid.h"

#include "logger.h"
#include "allocator.h"

#include <string.h>

#include <limits.h> /* for CHAR_BIT */

#define BITMASK(b) (1 << ((b) % CHAR_BIT))
#define BITSLOT(b) ((b) / CHAR_BIT)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + CHAR_BIT - 1) / CHAR_BIT)

Grid *createGrid(u64 size) {
    Grid *p = slMalloc(sizeof(Grid));
    p->data = slMalloc(sizeof(char) * BITNSLOTS(size));
    p->size = size;
    memset(p->data, 0, sizeof(bool) * p->size);

    return p;
}

void destroyGrid(Grid *p) {
    slFree(p->data);
    slFree(p);
}

void gridReset(Grid *p) {
    memset(p->data, 0, sizeof(char) * p->size);
}

bool gridGet(Grid *p, u64 pos) {
    return BITTEST(p->data, pos);
}

void gridSet(Grid *p, u64 pos, bool val) {
    if (val) {
        BITSET(p->data, pos);
    } else {
        BITCLEAR(p->data, pos);
    }
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
