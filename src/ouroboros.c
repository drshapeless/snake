#include "ouroboros.h"

#include "allocator.h"
#include "logger.h"

#include <strings.h>

Ouroboros *initOuroboros(u64 size) {
    Ouroboros *p = slMalloc(sizeof(Ouroboros));
    p->data = slMalloc(sizeof(u32) * size);
    p->max = size;
    resetOuroboros(p);

    return p;
}

void initOuroborosWithAllocatedMemory(Ouroboros *p, void *mem, u64 size) {
    p->data = mem;
    p->max = size;
    resetOuroboros(p);
}

void destroyOuroboros(Ouroboros *p) {
    slFree(p->data);
    slFree(p);
}

void resetOuroboros(Ouroboros *p) {
    p->len = 0;
    p->start = p->max / 2;
}

u32 *OuroborosAt(Ouroboros *p, u64 pos) {
    if (pos >= p->len) {
        ERROR("ouroboros pos > len")
        return NULL;
    }

    if (p->start + pos >= p->max) {
        return &p->data[pos - p->start];
    } else {
        return &p->data[p->start + pos];
    }
}

void OuroborosSet(Ouroboros *p, u64 pos, u32 val) {
    u32 *n = OuroborosAt(p, pos);
    if (n == NULL) {
        return;
    }

    *n = val;
}

void OuroborosPushBack(Ouroboros *p, u32 val) {
    if (p->len + 1 >= p->max) {
        ERROR("push back out of bound");
        return;
    }

    OuroborosSet(p, p->len, val);
    p->len += 1;
}

void OuroborosPushFront(Ouroboros *p, u32 val) {
    if (p->len + 1 >= p->max) {
        ERROR("push frount out of bound");
        return;
    }

    if (p->start - 1 < 0) {
        p->start = p->max - 1;
        p->len += 1;
        p->data[p->max - 1] = val;
    } else {
        p->start -= 1;
        p->len += 1;
        p->data[p->start] = val;
    }
}

u32 OuroborosPopBack(Ouroboros *p) {
    if (p->len - 1 < 0) {
        ERROR("pop back on empty ouroboros");
        return 0;
    }

    p->len -= 1;

    return *OuroborosAt(p, p->len);
}

u32 OuroborosPopFront(Ouroboros *p) {
    if (p->len - 1 < 0) {
        ERROR("pop front on empty ouroboros");
        return 0;
    }

    u32 v = *OuroborosAt(p, 0);

    if (p->start + 1 >= p->max) {
        p->start = 0;
        p->len -= 1;
    } else {
        p->start += 1;
        p->len -= 1;
    }

    return v;
}

u64 OuroborosArray(Ouroboros *p, u32 *arr) {
    if (p->start + p->len < p->max) {
        memcpy(arr, p->data + p->start, p->len);
    } else {
        u64 temp = p->max - p->start;
        memcpy(arr, p->data + p->start, temp);
        memcpy(arr + temp, p->data, p->len - temp);
    }

    return p->len;
}
