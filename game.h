#ifndef GAME_H
#define GAME_H

#include "defines.h"
#include "config.h"

struct Ouroboros {
    u32 *data;
    u64 max;
    u64 len;
    u64 start;
};

void init_ouroboros(struct Ouroboros *p, u64 size);
void destroy_ouroboros(struct Ouroboros *p);
void ouroboros_push_front(struct Ouroboros *p, u32 i);
void ouroboros_push_back(struct Ouroboros *p, u32 i);
u32 ouroboros_pop_back(struct Ouroboros *p);
u32 *ouroboros_at(struct Ouroboros *p, u64 pos);

struct Grids {
    bool *data;
    u64 max;
    u64 occupied;
};

void init_grids(struct Grids *p, u64 size);
void destroy_grids(struct Grids *p);
void grids_set_true(struct Grids *p, u64 pos);
void grids_set_false(struct Grids *p, u64 pos);

enum SnakeDirection {
    SNAKE_DIRECTION_UP = 0,
    SNAKE_DIRECTION_RIGHT = 1,
    SNAKE_DIRECTION_DOWN = 2,
    SNAKE_DIRECTION_LEFT = 3,
};

struct Game {
    struct Grids *grids;
    struct Ouroboros *snake;
    u32 apple;
    bool dead;
    enum SnakeDirection direction;
};

void init_game(struct Game *p);
void destroy_game(struct Game *p);
void game_change_direction(struct Game *p, enum SnakeDirection direction);
void game_tick(struct Game *p);

#endif /* GAME_H */
