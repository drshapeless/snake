#ifndef SNAKE_H
#define SNAKE_H

#include "ouroboros.h"
#include "grid.h"
#include "defines.h"
#include "renderer.h"

typedef enum Direction { UP = 0, LEFT, DOWN, RIGHT } Direction;

typedef struct Snake {
    Ouroboros *body;
    Grid *grid;
    u64 width;
    u64 height;
    bool is_dead;
    Direction direction;
    u64 apple;
} Snake;

Snake *createSnake(u64 w, u64 h);
void createSnakeWithAllocatedMemory(Snake *p, u64 w, u64 h, void *mem,
                                    u64 size);
void destroySnake(Snake *p);
void initSnake(Snake *p);
void snakeTick(Snake *p);
void snakeDraw(Snake *s, Renderer *r);

#endif /* SNAKE_H */
