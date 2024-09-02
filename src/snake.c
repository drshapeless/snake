#include "snake.h"

#include "allocator.h"

Snake *createSnake(u64 size, u64 w, u64 h) {
    Snake *p = slMalloc(sizeof(Snake));
    p->body = createOuroboros(size);
    p->grid = createGrid(size);
    p->width = w;
    p->height = h;
    p->is_dead = false;
    p->direction = LEFT;

    return p;
}

void createSnakeWithAllocatedMemory(Snake *p, u64 w, u64 h, void *mem,
                                    u64 size) {
    createOuroborosWithAllocatedMemory(p->body, mem, size);
    p->width = w;
    p->height = h;
    p->is_dead = false;
    p->direction = LEFT;
}

void destroySnake(Snake *p) {
    destroyOuroboros(p->body);
    slFree(p);
}

void generateNewApple(Snake *p) {
}

void tickSnake(Snake *p) {
    if (p->is_dead) {
        return;
    }

    u32 current_head = ouroborosHead(p->body);
    u32 next_head = 0;
    switch (p->direction) {
    case UP:
        if (current_head < p->width) {
            p->is_dead = true;
            return;
        } else {
            next_head = current_head - p->width;
        }
        break;
    case DOWN:
        if (current_head > p->width * (p->height - 1)) {
            p->is_dead = true;
            return;
        } else {
            next_head = current_head + p->width;
        }
        break;
    case LEFT:
        if (current_head % p->width == 0) {
            p->is_dead = true;
            return;
        } else {
            next_head = current_head - 1;
        }
        break;
    case RIGHT:
        if (current_head % p->width == p->width - 1) {
            p->is_dead = true;
            return;
        } else {
            next_head = current_head + 1;
        }
        break;
    }

    /* check collision with body */

    ouroborosPushFront(p->body, next_head);
    if (next_head == p->apple) {
        generateNewApple(p);
    } else {
        ouroborosPopBack(p->body);
    }
}
