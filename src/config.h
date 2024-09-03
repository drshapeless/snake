#ifndef CONFIG_H
#define CONFIG_H

#include "defines.h"

static const char WINDOW_TITLE[] = "snake";
static const u64 WINDOW_WIDTH = 800;
static const u64 WINDOW_HEIGHT = 600;

static const u64 SQUARE_SIZE = 20;

static const SDL_Color COLOR_BLACK = { 0, 0, 0, 255 };
static const SDL_Color COLOR_GRAY = { 100, 100, 100, 255 };
static const SDL_Color COLOR_WHITE = { 255, 255, 255, 255 };
static const SDL_Color COLOR_RED = { 255, 0, 0, 255 };

#endif /* CONFIG_H */
