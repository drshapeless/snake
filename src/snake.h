#ifndef SNAKE_H
#define SNAKE_H

#include "ouroboros.h"
#include "grid.h"
#include "defines.h"

typedef enum Direction { UP, DOWN, LEFT, RIGHT } Direction;

typedef struct Snake {
    Ouroboros *body;
    Grid *grid;
    u64 width;
    u64 height;
    bool is_dead;
    Direction direction;
    u32 apple;
} Snake;

Snake *createSnake(u64 size, u64 w, u64 h);
void createSnakeWithAllocatedMemory(Snake *p, u64 w, u64 h, void *mem,
                                    u64 size);
void destroySnake(Snake *p);
void tickSnake(Snake *p);

#endif /* SNAKE_H */
