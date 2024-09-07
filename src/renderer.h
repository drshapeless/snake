#ifndef RENDERER_H
#define RENDERER_H

#include <SDL3/SDL.h>
#include "defines.h"

typedef struct Renderer {
    SDL_Renderer *r;
    u64 frame_number;
} Renderer;

Renderer *createRenderer(SDL_Window *w);
void destroyRenderer(Renderer *p);
void rendererSetDrawColor(Renderer *p, SDL_Color color);
void rendererDrawRect(Renderer *p, SDL_FRect *rect);
void rendererFillRect(Renderer *p, SDL_FRect *rect);
void rendererFillRects(Renderer *p, SDL_FRect *rects, u64 len);
void rendererClear(Renderer *p);
void rendererPresent(Renderer *p);

void rendererBeginDraw(Renderer *p);
void rendererEndDraw(Renderer *p);

#endif /* RENDERER_H */
