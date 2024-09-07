#include "ouroboros.h"

#include "allocator.h"
#include "logger.h"

#include <strings.h>

Ouroboros *createOuroboros(u64 size) {
    Ouroboros *p = slAlloc(sizeof(Ouroboros));
    p->data = slAlloc(sizeof(u64) * size);
    p->max = size;
    ouroborosReset(p);

    return p;
}

void createOuroborosWithAllocatedMemory(Ouroboros *p, void *mem, u64 size) {
    p->data = mem;
    p->max = size;
    ouroborosReset(p);
}

void destroyOuroboros(Ouroboros *p) {
    slFree(p->data);
    slFree(p);
}

void ouroborosReset(Ouroboros *p) {
    p->len = 0;
    p->start = p->max / 2;
}

u64 *ouroborosAt(Ouroboros *p, u64 pos) {
    if (pos >= p->len) {
        ERROR("ouroboros pos > len, pos %llu, len %llu", pos, p->len);
        return NULL;
    }

    if (p->start + pos >= p->max) {
        return &p->data[pos - p->start];
    } else {
        return &p->data[p->start + pos];
    }
}

void ouroborosSet(Ouroboros *p, u64 pos, u64 val) {
    u64 *n = ouroborosAt(p, pos);
    if (n == NULL) {
        return;
    }

    *n = val;
}

void ouroborosPushBack(Ouroboros *p, u64 val) {
    if (p->len + 1 >= p->max) {
        ERROR("push back out of bound");
        return;
    }

    p->len += 1;
    ouroborosSet(p, p->len - 1, val);
}

void ouroborosPushFront(Ouroboros *p, u64 val) {
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

u64 ouroborosPopBack(Ouroboros *p) {
    if (p->len - 1 < 0) {
        ERROR("pop back on empty ouroboros");
        return 0;
    }

    u64 temp = *ouroborosAt(p, p->len - 1);

    p->len -= 1;

    return temp;
}

u64 ouroborosPopFront(Ouroboros *p) {
    if (p->len - 1 < 0) {
        ERROR("pop front on empty ouroboros");
        return 0;
    }

    u64 v = *ouroborosAt(p, 0);

    if (p->start + 1 >= p->max) {
        p->start = 0;
        p->len -= 1;
    } else {
        p->start += 1;
        p->len -= 1;
    }

    return v;
}

u64 ouroborosArray(Ouroboros *p, u64 *arr) {
    if (p->len == 0) {
        ERROR("empty ouroboros");
        return 0;
    }

    if (p->start + p->len < p->max) {
        memcpy(arr, p->data + p->start, p->len * 8);
    } else {
        u64 temp = p->max - p->start;
        memcpy(arr, p->data + p->start, temp * 8);
        memcpy(arr + temp, p->data, p->len - temp * 8);
    }

    /* /\* use a dumb way to do this *\/ */
    /* for (u64 i = 0; i < p->len; i++) { */
    /*     arr[i] = *ouroborosAt(p, i); */
    /* } */

    return p->len;
}

u64 ouroborosHead(Ouroboros *p) {
    return *ouroborosAt(p, 0);
}
