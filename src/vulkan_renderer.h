#ifndef VULKAN_RENDERER_H
#define VULKAN_RENDERER_H

#include <vulkan/vulkan.h>

#include "defines.h"
#include "config.h"
#include "slassert.h"

#define VK_CHECK(expr) slassert(expr == VK_SUCCESS)

typedef struct VulkanContext {
    VkInstance instance;
    VkAllocationCallbacks *allocator;
#ifdef DEBUG
    VkDebugUtilsMessengerEXT debugMessenger;
#endif
} VulkanContext;

typedef struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR *formats;
    u32 formatCount;
    VkPresentModeKHR *presentModes;
    u32 presentModeCount;
} SwapChainSupportDetails;

bool initVulkanContext(VulkanContext *context);
void destroyVulkanContext(VulkanContext *context);

#endif /* VULKAN_RENDERER_H */
