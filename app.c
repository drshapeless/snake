#include "app.h"
#include "config.h"
#include "logger.h"
#include "game.h"

#include <stdlib.h>

void init_app(struct App *p) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        FATAL(SDL_GetError());
        exit(1);
    }

    p->window = SDL_CreateWindow(WINDOW_TITLE, SCREEN_WIDTH, SCREEN_HEIGHT,
                                 SDL_WINDOW_RESIZABLE);
    if (p->window == NULL) {
        FATAL("cannot create window %s", SDL_GetError());
        exit(1);
    }

    p->renderer = SDL_CreateRenderer(p->window, NULL, SDL_RENDERER_ACCELERATED);
    if (p->renderer == NULL) {
        FATAL("cannot create renderer %s", SDL_GetError());
        exit(1);
    }

    p->game = malloc(sizeof(struct Game));
    init_game(p->game);
}

void destroy_app(struct App *p) {
    destroy_game(p->game);
    free(p->game);
    SDL_DestroyRenderer(p->renderer);
    SDL_DestroyWindow(p->window);
    SDL_Quit();
}

SDL_FRect pos_to_rect(u32 pos) {
    SDL_FRect rect = { 0 };
    rect.w = SCREEN_WIDTH / GRID_WIDTH;
    rect.h = SCREEN_HEIGHT / GRID_HEIGHT;
    rect.x = pos % GRID_WIDTH * rect.w;
    rect.y = pos / GRID_WIDTH * rect.h;

    rect.w -= 2;
    rect.h -= 2;
    rect.x += 1;
    rect.y += 1;

    return rect;
}

void app_run(struct App *p) {
    bool running = true;
    bool pause = false;
    enum SnakeDirection new_direction = SNAKE_DIRECTION_LEFT;
    u64 last_tick = 0;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            case SDL_EVENT_KEY_DOWN:
                switch (e.key.keysym.sym) {
                case SDLK_w:
                    if (!pause) {
                        new_direction = SNAKE_DIRECTION_UP;
                    }
                    break;
                case SDLK_d:
                    if (!pause) {
                        new_direction = SNAKE_DIRECTION_RIGHT;
                    }
                    break;
                case SDLK_a:
                    if (!pause) {
                        new_direction = SNAKE_DIRECTION_LEFT;
                    }
                    break;
                case SDLK_s:
                    if (!pause) {
                        new_direction = SNAKE_DIRECTION_DOWN;
                    }
                    break;
                case SDLK_SPACE:
                    if (p->game->dead) {
                        init_game(p->game);
                    } else {
                        pause = !pause;
                    }
                    break;
                case SDLK_ESCAPE:
                    running = false;
                    break;
                }
                break;
            }
        }

        if (p->game->dead || pause) {
            continue;
        }

        u64 current = SDL_GetTicks();
        u64 delta = current - last_tick;
        if (delta < 1000 / FPS) {
            continue;
        }
        last_tick = current;

        game_change_direction(p->game, new_direction);
        game_tick(p->game);

        SDL_SetRenderDrawColor(p->renderer, 0, 0, 0, 255);
        SDL_RenderClear(p->renderer);

        SDL_SetRenderDrawColor(p->renderer, 255, 0, 0, 255);
        SDL_FRect apple = pos_to_rect(p->game->apple);
        SDL_RenderFillRect(p->renderer, &apple);

        if (p->game->dead) {
            SDL_SetRenderDrawColor(p->renderer, 150, 150, 150, 255);
        } else {
            SDL_SetRenderDrawColor(p->renderer, 255, 255, 255, 255);
        }

        SDL_FRect body[p->game->snake->len];

        for (int i = 0; i < p->game->snake->len; i++) {
            u32 block = *ouroboros_at(p->game->snake, i);
            body[i] = pos_to_rect(block);
        }
        SDL_RenderFillRects(p->renderer, body, p->game->snake->len);
        SDL_RenderPresent(p->renderer);
    }
}
