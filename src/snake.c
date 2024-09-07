#include "snake.h"

#include "allocator.h"
#include "config.h"
#include "logger.h"

Snake *createSnake(u64 w, u64 h) {
    Snake *p = slAlloc(sizeof(Snake));
    p->body = createOuroboros(w * h);
    p->grid = createGrid(w * h);
    p->width = w;
    p->height = h;
    initSnake(p);
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
    destroyGrid(p->grid);
    destroyOuroboros(p->body);
    slFree(p);
}

void generateNewApple(Snake *p) {
    u64 arr[p->grid->size];

    u64 empty_grid_count = gridEmptyArray(p->grid, arr);
    u64 temp = SDL_rand(empty_grid_count);
    p->apple = arr[temp];
}

void initSnake(Snake *p) {
    ouroborosReset(p->body);
    gridReset(p->grid);
    p->is_dead = false;
    p->direction = LEFT;

    u64 i = p->width * p->height / 2 + p->width / 2;
    u64 temp[3] = { i, i + 1, i + 2 };
    for (u64 j = 0; j < 3; j++) {
        ouroborosPushBack(p->body, temp[j]);
        gridSet(p->grid, temp[j], true);
    }

    generateNewApple(p);
}

void snakeTick(Snake *p) {
    if (p->is_dead) {
        return;
    }

    u64 current_head = ouroborosHead(p->body);
    u64 next_head = 0;

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
    if (gridGet(p->grid, next_head)) {
        /* die */
        p->is_dead = true;
        INFO("BANG");
        return;
    }

    gridSet(p->grid, next_head, true);

    ouroborosPushFront(p->body, next_head);
    if (next_head == p->apple) {
        generateNewApple(p);
    } else {
        u64 temp = ouroborosPopBack(p->body);
        gridSet(p->grid, temp, false);
    }
}

SDL_FRect posToRect(u64 w, u64 pos) {
    u64 y = pos / w;
    u64 x = pos % w;

    SDL_FRect rect = { x * 20, y * 20, SQUARE_SIZE, SQUARE_SIZE };
    return rect;
}

void drawApple(Snake *snake, Renderer *r) {
    rendererSetDrawColor(r, COLOR_RED);
    SDL_FRect rect = posToRect(snake->width, snake->apple);
    rendererFillRect(r, &rect);
}

void drawBody(Snake *s, Renderer *r) {
    u64 arr[s->width * s->height];
    u64 len = ouroborosArray(s->body, arr);

    SDL_FRect rects[len];
    for (u64 i = 0; i < len; i++) {
        u64 pos = arr[i];
        rects[i] = posToRect(s->width, pos);
    }

    if (s->is_dead) {
        rendererSetDrawColor(r, COLOR_GRAY);
    } else {
        rendererSetDrawColor(r, COLOR_WHITE);
    }

    rendererFillRects(r, rects, len);
}

void snakeDraw(Snake *s, Renderer *r) {
    drawApple(s, r);
    drawBody(s, r);
}
