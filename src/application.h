#ifndef APPLICATION_H
#define APPLICATION_H

#include <SDL3/SDL.h>

#include "renderer.h"
#include "snake.h"

typedef struct Application {
    SDL_Window *window;
    Renderer *renderer;
    Snake *snake;
} Application;

Application *createApplication();
void destroyApplication(Application *app);
void applicationRun(Application *app);

#endif /* APPLICATION_H */
