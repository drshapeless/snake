#ifndef LOGGER_H
#define LOGGER_H

#include <SDL3/SDL.h>

#define ERROR(...) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__);
#define DEBUG(...) SDL_LogDebug(int category, const char *fmt, ...);
#define INFO(...) SDL_LogInfo(int category, const char *fmt, ...);

#endif /* LOGGER_H */
