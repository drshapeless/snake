#ifndef GRID_H
#define GRID_H

#include "defines.h"

/* This holds a bunch of bool in bit */
/* The data will be aligned with byte*/
typedef struct Grid {
    char *data;
    u64 size;
} Grid;

/* Size is how many bool */
Grid *createGrid(u64 size);
void destroyGrid(Grid *p);

void gridReset(Grid *p);
void gridSet(Grid *p, u64 pos, bool val);
bool gridGet(Grid *p, u64 pos);
u64 gridEmptyArray(Grid *p, u64 *arr);
u64 gridArray(Grid *p, u64 *arr);

#endif /* GRID_H */
