#include <SDL3/SDL.h>

#include "logger.h"
#include "defines.h"
#include "snake.h"
#include "renderer.h"
#include "config.h"

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        ERROR("%s", SDL_GetError());
        return 1;
    }

    SDL_SetLogPriorities(SDL_LOG_PRIORITY_INFO);

    SDL_Window *window = SDL_CreateWindow(WINDOW_TITLE, WINDOW_WIDTH,
                                          WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (window == NULL) {
        ERROR("%s", SDL_GetError());
        return 1;
    }

    Renderer *renderer = createRenderer(window);

    Snake *snake =
        createSnake(WINDOW_WIDTH / SQUARE_SIZE, WINDOW_HEIGHT / SQUARE_SIZE);

    u64 last_time = SDL_GetTicks();
    Direction next_direction = snake->direction;

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
                    if (snake->is_dead) {
                        initSnake(snake);
                    }
                }
                break;
            }
        }

        u64 current_time = SDL_GetTicks();
        if (current_time - last_time < 100) {
            continue;
        }

        last_time = current_time;
        if (next_direction % 2 != snake->direction % 2) {
            snake->direction = next_direction;
        }

        snakeTick(snake);

        rendererSetDrawColor(renderer, COLOR_BLACK);
        rendererClear(renderer);

        rendererSetDrawColor(renderer, COLOR_WHITE);

        snakeDraw(snake, renderer);

        SDL_FRect outline = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
        rendererDrawRect(renderer, &outline);

        rendererPresent(renderer);
    }

    destroySnake(snake);
    destroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
