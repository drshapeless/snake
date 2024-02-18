#include "game.h"
#include "logger.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

void init_ouroboros(struct Ouroboros *p, u64 size) {
    p->data = (u32 *)malloc(sizeof(u32) * size);
    p->max = size;
    p->len = 0;
    p->start = 10;
}

void destroy_ouroboros(struct Ouroboros *p) {
    free(p->data);
}

void ouroboros_push_front(struct Ouroboros *p, u32 i) {
    if (p->start == 0) {
        p->start = p->max - 1;
    } else {
        p->start -= 1;
    }

    p->data[p->start] = i;
    p->len += 1;
}

void ouroboros_push_back(struct Ouroboros *p, u32 i) {
    *ouroboros_at(p, p->len) = i;

    p->len += 1;
}

/* iterating the struct using this function is slow */
u32 *ouroboros_at(struct Ouroboros *p, u64 pos) {
    int i;
    if (p->start + pos >= p->max) {
        i = p->start + pos - p->max;

    } else {
        i = p->start + pos;
    }

    return &p->data[i];
}

u32 ouroboros_pop_back(struct Ouroboros *p) {
    p->len -= 1;
    return *ouroboros_at(p, p->len);
}

void init_grids(struct Grids *p, u64 size) {
    p->data = (bool *)malloc(sizeof(bool) * size);
    memset(p->data, 0, sizeof(bool) * size);
    p->max = size;
    p->occupied = 0;
}

void destroy_grids(struct Grids *p) {
    free(p->data);
}

void grids_set_true(struct Grids *p, u64 pos) {
    if (pos < p->max) {
        p->data[pos] = true;
        p->occupied += 1;
    }
}

void grids_set_false(struct Grids *p, u64 pos) {
    if (pos < p->max && p->occupied > 0) {
        p->data[pos] = false;
        p->occupied -= 1;
    }
}

bool grids_at(struct Grids *p, u64 pos) {
    return p->data[pos];
}

u32 generate_apple(struct Grids *g) {
    int a = rand() % (g->max - g->occupied);

    int counter = -1;
    for (int i = 0; i < g->max; i++) {
        if (g->data[i] == false) {
            counter++;
        }
        if (counter == a) {
            return i;
        }
    }

    return 0;
}

void init_game(struct Game *p) {
    p->grids = malloc(sizeof(struct Grids));
    init_grids(p->grids, GRID_COUNT);
    p->snake = malloc(sizeof(struct Ouroboros));
    init_ouroboros(p->snake, GRID_COUNT);
    p->dead = false;
    p->direction = SNAKE_DIRECTION_LEFT;

    srand(time(NULL));
    /* set initial snake */
    for (int i = 0; i < 3; i++) {
        int j = GRID_COUNT / 2 + GRID_WIDTH / 2 + i;
        ouroboros_push_back(p->snake, j);
        grids_set_true(p->grids, j);
    }

    p->apple = generate_apple(p->grids);
}

void destroy_game(struct Game *p) {
    destroy_ouroboros(p->snake);
    free(p->snake);
    destroy_grids(p->grids);
    free(p->grids);
}

void game_change_direction(struct Game *p, enum SnakeDirection direction) {
    if ((p->direction + direction) % 2 > 0) {
        p->direction = direction;
    }
}

void game_tick(struct Game *p) {
    u32 new_head;
    u32 head = *ouroboros_at(p->snake, 0);
    switch (p->direction) {
    case SNAKE_DIRECTION_UP:
        if (head < GRID_WIDTH) {
            p->dead = true;
            return;
        }
        new_head = head - GRID_WIDTH;
        break;
    case SNAKE_DIRECTION_DOWN:
        if (head >= (GRID_WIDTH * (GRID_HEIGHT - 1))) {
            p->dead = true;
            return;
        }

        new_head = head + GRID_WIDTH;
        break;
    case SNAKE_DIRECTION_LEFT:
        if ((head % GRID_WIDTH) == 0) {
            p->dead = true;
            return;
        }
        new_head = head - 1;
        break;
    case SNAKE_DIRECTION_RIGHT:
        if ((head % GRID_WIDTH) == GRID_WIDTH - 1) {
            p->dead = true;
            return;
        }
        new_head = head + 1;
        break;
    }

    if (grids_at(p->grids, new_head)) {
        p->dead = true;
        return;
    }

    ouroboros_push_front(p->snake, new_head);
    grids_set_true(p->grids, new_head);

    if (new_head == p->apple) {
        p->apple = generate_apple(p->grids);
    } else {
        u32 tail = ouroboros_pop_back(p->snake);
        grids_set_false(p->grids, tail);
    }
}
