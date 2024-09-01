#ifndef OUROBOROS_H
#define OUROBOROS_H

#include "defines.h"
typedef struct Ouroboros {
    u32 *data;
    u64 max;
    u64 len;
    u64 start;
} Ouroboros;

Ouroboros *initOuroboros(u64 size);
void initOuroborosWithAllocatedMemory(Ouroboros *p, void *mem, u64 size);
void destroyOuroboros(Ouroboros *p);
void resetOuroboros(Ouroboros *p);
void OuroborosPushBack(Ouroboros *p, u32 val);
void OuroborosPushFront(Ouroboros *p, u32 val);

/* Return the value of the removed data. */
u32 OuroborosPopBack(Ouroboros *p);

/* Return the value of the removed data. */
u32 OuroborosPopFront(Ouroboros *p);

/* Assume the memory is sufficient, no checking. */
/* Return the len of the array. */
u64 OuroborosArray(Ouroboros *p, u32 *arr);

#endif /* OUROBOROS_H */
