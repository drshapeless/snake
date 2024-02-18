#ifndef APP_H
#define APP_H

#include <SDL3/SDL.h>

struct App {
    SDL_Window *window;
    SDL_Renderer *renderer;
    struct Game *game;
};

void init_app(struct App *p);
void destroy_app(struct App *p);
void app_run(struct App *p);

#endif /* APP_H */
