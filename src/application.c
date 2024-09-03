#include "application.h"

#include "logger.h"
#include "allocator.h"
#include "config.h"

#include <stdlib.h>

void initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        ERROR("%s", SDL_GetError());
        exit(1);
    }

    SDL_SetLogPriorities(SDL_LOG_PRIORITY_INFO);
}

Application *createApplication() {
    initSDL();
    Application *app = slMalloc(sizeof(Application));
    app->window = SDL_CreateWindow(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT,
                                   SDL_WINDOW_RESIZABLE);

    if (app->window == NULL) {
        ERROR("%s", SDL_GetError());
        exit(1);
    }
    app->renderer = createRenderer(app->window);
    app->snake =
        createSnake(WINDOW_WIDTH / SQUARE_SIZE, WINDOW_HEIGHT / SQUARE_SIZE);

    return app;
}

void destroyApplication(Application *app) {
    destroySnake(app->snake);
    destroyRenderer(app->renderer);
    SDL_DestroyWindow(app->window);
    SDL_Quit();
    slFree(app);
}

void applicationRun(Application *app) {
    bool pause = false;
    u64 last_time = SDL_GetTicks();
    Direction next_direction = app->snake->direction;

    bool running = true;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            case SDL_EVENT_KEY_DOWN:
                switch (e.key.key) {
                case SDLK_ESCAPE:
                    running = false;
                    break;
                case SDLK_W:
                    next_direction = UP;
                    break;
                case SDLK_A:
                    next_direction = LEFT;
                    break;
                case SDLK_S:
                    next_direction = DOWN;
                    break;
                case SDLK_D:
                    next_direction = RIGHT;
                    break;
                case SDLK_SPACE:
                    if (app->snake->is_dead) {
                        initSnake(app->snake);
                    } else {
                        pause = !pause;
                    }
                }
                break;
            }
        }

        if (pause) {
            continue;
        }

        u64 current_time = SDL_GetTicks();
        if (current_time - last_time < 100) {
            continue;
        }

        last_time = current_time;
        if (next_direction % 2 != app->snake->direction % 2) {
            app->snake->direction = next_direction;
        }

        snakeTick(app->snake);

        rendererSetDrawColor(app->renderer, COLOR_BLACK);
        rendererClear(app->renderer);

        rendererSetDrawColor(app->renderer, COLOR_WHITE);

        snakeDraw(app->snake, app->renderer);

        SDL_FRect outline = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
        rendererDrawRect(app->renderer, &outline);

        rendererPresent(app->renderer);
    }
}
