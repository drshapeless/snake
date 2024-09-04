#include "vulkan_engine.h"

#include <stdlib.h>
#include <SDL3/SDL_vulkan.h>

#include "logger.h"
#include "allocator.h"
#include "config.h"

VulkanEngine *createVulkanEngine(SDL_Window *window) {
    VulkanEngine *engine = slMalloc(sizeof(VulkanEngine));
    engine->window = window;
    initVulkan(engine);
    return engine;
}

void destroyVulkanEngine(VulkanEngine *engine) {
    vkDestroyDevice(engine->device, NULL);
    vkDestroyInstance(engine->instance, NULL);
    slFree(engine);
}

void createInstance(VkInstance *instance);
void selectPhysicalDevice(VulkanEngine *engine);
void createLogicalDevice(VulkanEngine *engine);
void createSurface(VulkanEngine *e);

void initVulkan(VulkanEngine *e) {
    createInstance(&e->instance);
    selectPhysicalDevice(e);
    createLogicalDevice(e);
    createSurface(e);
}

void createInstance(VkInstance *instance) {
    /* TODO: check validation layer support */

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = WINDOW_TITLE;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Shapeless Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    /* TODO: extensions support */
    /* TODO: debug messenger */

    if (vkCreateInstance(&createInfo, NULL, instance) != VK_SUCCESS) {
        ERROR("Failed to create Vulkan instance");
        exit(EXIT_FAILURE);
    }
}

void selectPhysicalDevice(VulkanEngine *e) {
    u32 device_count = 0;
    vkEnumeratePhysicalDevices(e->instance, &device_count, NULL);
    if (device_count == 0) {
        ERROR("Failed to find GPUs with Vulkan support!");
        exit(EXIT_FAILURE);
    }

    VkPhysicalDevice *devices = malloc(sizeof(VkPhysicalDevice) * device_count);
    vkEnumeratePhysicalDevices(e->instance, &device_count, devices);

    /* loop the device, select with desire feature */
    /* TODO: omit it for now, select the first gpu */
    e->physical_device = devices[0];

    free(devices);
    if (e->physical_device == VK_NULL_HANDLE) {
        ERROR("Failed to find a suitable GPU!");
        exit(EXIT_FAILURE);
    }
}

void createLogicalDevice(VulkanEngine *e) {
    /* TODO: find queue families here */

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = 0; // Set the correct index
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures = {}; // Set features if needed
    /* TODO: set features here */

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.pEnabledFeatures = &deviceFeatures;

    if (vkCreateDevice(e->physical_device, &createInfo, NULL, &e->device) !=
        VK_SUCCESS) {
        ERROR("Failed to create logical device!");
        exit(EXIT_FAILURE);
    }
}

void createSurface(VulkanEngine *e) {
    if (SDL_Vulkan_CreateSurface(e->window, e->instance, NULL, &e->surface)) {
        ERROR("Failed to create window surface!");
        exit(EXIT_FAILURE);
    }
}
