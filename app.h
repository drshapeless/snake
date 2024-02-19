#ifndef APP_H
#define APP_H

#include "defines.h"
#include "game.h"

#include <SDL3/SDL.h>

struct App {
    SDL_Window *window;
    SDL_Renderer *renderer;
    struct Game *game;
    bool running;
    bool pause;
    u64 last_tick;
    enum SnakeDirection new_direction;
};

struct App *create_app(void);
void init_app(struct App *p);
void destroy_app(struct App *p);
void app_run(struct App *p);

#endif /* APP_H */
