#ifndef OUROBOROS_H
#define OUROBOROS_H

#include "defines.h"
typedef struct Ouroboros {
    u32 *data;
    u64 max;
    u64 len;
    u64 start;
} Ouroboros;

Ouroboros *createOuroboros(u64 size);
void createOuroborosWithAllocatedMemory(Ouroboros *p, void *mem, u64 size);
void destroyOuroboros(Ouroboros *p);
void resetOuroboros(Ouroboros *p);
void ouroborosPushBack(Ouroboros *p, u32 val);
void ouroborosPushFront(Ouroboros *p, u32 val);

/* Return the value of the removed data. */
u32 ouroborosPopBack(Ouroboros *p);

/* Return the value of the removed data. */
u32 ouroborosPopFront(Ouroboros *p);

/* Assume the memory is sufficient, no checking. */
/* Return the len of the array. */
u64 ouroborosArray(Ouroboros *p, u32 *arr);

u32 ouroborosHead(Ouroboros *p);

#endif /* OUROBOROS_H */
