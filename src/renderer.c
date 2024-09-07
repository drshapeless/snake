#include "renderer.h"

#include "logger.h"
#include "allocator.h"

#include <stdlib.h>

Renderer *createRenderer(SDL_Window *window) {
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == NULL) {
        ERROR("%s", SDL_GetError());
        exit(EXIT_FAILURE);
        return NULL;
    }

    Renderer *p = slAlloc(sizeof(Renderer));
    p->r = renderer;
    p->frame_number = 0;
    return p;
}

void destroyRenderer(Renderer *p) {
    SDL_DestroyRenderer(p->r);
    slFree(p);
}

void rendererSetDrawColor(Renderer *p, SDL_Color color) {
    SDL_SetRenderDrawColor(p->r, color.r, color.g, color.b, color.a);
}

void rendererFillRect(Renderer *r, SDL_FRect *rect) {
    SDL_RenderFillRect(r->r, rect);
}

void rendererDrawRect(Renderer *r, SDL_FRect *rect) {
    SDL_RenderRect(r->r, rect);
}

void rendererFillRects(Renderer *p, SDL_FRect *rects, u64 len) {
    SDL_RenderFillRects(p->r, rects, len);
}

void rendererClear(Renderer *p) {
    SDL_RenderClear(p->r);
}

void rendererPresent(Renderer *p) {
    SDL_RenderPresent(p->r);
}

void rendererEndDraw(Renderer *p) {
    p->frame_number += 1;
}
