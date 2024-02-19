#include "app.h"
#include "config.h"
#include "logger.h"
#include "game.h"
#include "allocator.h"

#include <stdlib.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

struct App *create_app(void) {
    struct App *p = (struct App *)salloc(sizeof(struct App));
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

    p->game = create_game();
    game_init(p->game);

    p->running = true;
    p->pause = false;
    p->last_tick = 0;
    p->new_direction = SNAKE_DIRECTION_LEFT;
    return p;
}

void destroy_app(struct App *p) {
    destroy_game(p->game);
    SDL_DestroyRenderer(p->renderer);
    SDL_DestroyWindow(p->window);
    SDL_Quit();
    sfree(p);
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

void app_process_input(struct App *p) {
    enum SnakeDirection local_direction = p->new_direction;
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_EVENT_QUIT:
            p->running = false;
            break;
        case SDL_EVENT_KEY_DOWN:
            switch (e.key.keysym.sym) {
            case SDLK_w:
                local_direction = SNAKE_DIRECTION_UP;
                break;
            case SDLK_d:
                local_direction = SNAKE_DIRECTION_RIGHT;
                break;
            case SDLK_a:
                local_direction = SNAKE_DIRECTION_LEFT;
                break;
            case SDLK_s:
                local_direction = SNAKE_DIRECTION_DOWN;
                break;
            case SDLK_SPACE:
                if (p->game->dead) {
                    game_reset(p->game);
                    local_direction = SNAKE_DIRECTION_LEFT;
                } else {
                    p->pause = !p->pause;
                }
                break;
            case SDLK_ESCAPE:
                p->running = false;
                break;
            }
            break;
        }
    }
    if (!p->pause) {
        p->new_direction = local_direction;
    }
    if (p->game->dead || p->pause) {
        return;
    }
}

void app_draw(struct App *p) {
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

void app_mainloop(struct App *p) {
    while (p->running) {
        app_process_input(p);

        if (p->game->dead || p->pause) {
            continue;
        }

        u64 current = SDL_GetTicks();
        u64 delta = current - p->last_tick;
        if (delta < 1000 / FPS) {
            continue;
        }
        p->last_tick = current;
        game_change_direction(p->game, p->new_direction);
        game_tick(p->game);

        app_draw(p);
    }
}

void app_wasm_mainloop(void *args_) {
    struct App *p = (struct App *)args_;
    if (p == NULL) {
        FATAL("big trouble");
    }
    if (!p->running) {
        destroy_app(p);
#ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop(); /* this should "kill" the app. */
#endif
    }
    app_process_input(p);
    if (p->pause || p->game->dead) {
        return;
    }
    game_change_direction(p->game, p->new_direction);
    game_tick(p->game);
    app_draw(p);
}

void app_run(struct App *p) {
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(app_wasm_mainloop, p, FPS, 1);
#else
    app_mainloop(p);
#endif
}
