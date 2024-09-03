#ifndef LOGGER_H
#define LOGGER_H

#include <SDL3/SDL.h>

#define ERROR(...) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__);
#define DEBUG(...) SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__);
#define INFO(...) SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__);

#endif /* LOGGER_H */
