#ifndef VULKAN_ENGINE_H
#define VULKAN_ENGINE_H

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>

typedef struct VulkanEngine {
    SDL_Window *window;
    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkSurfaceKHR surface;
} VulkanEngine;

VulkanEngine *createVulkanEngine(SDL_Window *window);
void destroyVulkanEngine(VulkanEngine *engine);

void initVulkan(VulkanEngine *engine);

#endif /* VULKAN_ENGINE_H */
