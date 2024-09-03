#ifndef OUROBOROS_H
#define OUROBOROS_H

#include "defines.h"
typedef struct Ouroboros {
    u64 *data;
    u64 max;
    u64 len;
    u64 start;
} Ouroboros;

Ouroboros *createOuroboros(u64 size);
void createOuroborosWithAllocatedMemory(Ouroboros *p, void *mem, u64 size);
void destroyOuroboros(Ouroboros *p);
void ouroborosReset(Ouroboros *p);
void ouroborosPushBack(Ouroboros *p, u64 val);
void ouroborosPushFront(Ouroboros *p, u64 val);

/* Return the value of the removed data. */
u64 ouroborosPopBack(Ouroboros *p);

/* Return the value of the removed data. */
u64 ouroborosPopFront(Ouroboros *p);

/* Assume the memory is sufficient, no checking. */
/* Return the len of the array. */
u64 ouroborosArray(Ouroboros *p, u64 *arr);

u64 ouroborosHead(Ouroboros *p);

#endif /* OUROBOROS_H */
