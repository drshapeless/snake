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

/* Vulkan engine */
#include <vulkan/vulkan.h>
#ifdef NDEBUG
static const bool enableValidationLayers = false;
#else
static const bool enableValidationLayers = true;
#endif

static const char *validationLayers[] = { "VK_LAYER_KHRONOS_validation" };
static const i32 validationLayerCount = 1;

#ifdef __APPLE__
#include <vulkan/vulkan_beta.h>
/* dirty fix for moltenvk */
static const char *deviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
};
static const i32 deviceExtensionCount = 2;
#else
static const char *deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
static const i32 deviceExtensionCount = 1;
#endif

static const i32 MAX_FRAMES_IN_FLIGHT = 2;

#endif /* CONFIG_H */
