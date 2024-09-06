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
    createSurface(e);
    selectPhysicalDevice(e);
    createLogicalDevice(e);
}

bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    VkLayerProperties *availableLayers =
        slMalloc(sizeof(VkLayerProperties) * layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    bool isSupport = true;

    for (int i = 0; i < validationLayerCount; i++) {
        bool layerFound = false;
        for (int j = 0; j < layerCount; j++) {
            if (strcmp(validationLayers[i], availableLayers[j].layerName) ==
                0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            isSupport = false;
            break;
        }
    }

    slFree(availableLayers);

    return isSupport;
}

const char **getRequiredExtensions(u32 *extension_count) {
    u32 sdlExtensionCount = 0;
    const char *const *sdlExtensions;

    sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);

    *extension_count = sdlExtensionCount;
    const char **extensions = slMalloc(sizeof(char **) * sdlExtensionCount);

    if (enableValidationLayers) {
        extensions = slRealloc(extensions, *extension_count + 1);
        extensions[*extension_count] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        *extension_count += 1;
    }

    /* Not sure if this is still needed for macOS */
    /* #ifdef __APPLE__ */
    /*     extensions = slRealloc(extensions, *extension_count + 1); */
    /*     extensions[*extensionCount] = */
    /*         VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME; */
    /*     *extensionCount += 1; */
    /* #endif */

    return extensions;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
              void *pUserData) {
    DEBUG("validation layer: %s", pCallbackData->pMessage);

    return VK_FALSE;
}

void populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT *createInfo) {
    createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo->messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo->pfnUserCallback = debugCallback;
}

void createInstance(VkInstance *instance) {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        ERROR("validation layers requested, but not available!");
        exit(EXIT_FAILURE);
    }

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

    u32 extensionCount = 0;
    const char **extensions = getRequiredExtensions(&extensionCount);
    createInfo.enabledExtensionCount = extensionCount;
    createInfo.ppEnabledExtensionNames = extensions;
#ifdef __APPLE__
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = { 0 };
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = validationLayerCount;
        createInfo.ppEnabledLayerNames = validationLayers;
        populateDebugMessengerCreateInfo(&debugCreateInfo);
        createInfo.pNext =
            (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = NULL;
    }

    if (vkCreateInstance(&createInfo, NULL, instance) != VK_SUCCESS) {
        ERROR("Failed to create Vulkan instance");
        exit(EXIT_FAILURE);
    }
}

void selectPhysicalDevice(VulkanEngine *e) {
    u32 deviceCount = 0;
    vkEnumeratePhysicalDevices(e->instance, &deviceCount, NULL);
    if (deviceCount == 0) {
        ERROR("Failed to find GPUs with Vulkan support!");
        exit(EXIT_FAILURE);
    }

    VkPhysicalDevice *devices = malloc(sizeof(VkPhysicalDevice) * deviceCount);
    vkEnumeratePhysicalDevices(e->instance, &deviceCount, devices);

    /* loop the device, select with desire feature */
    /* TODO: omit it for now, select the first gpu */
    e->physicalDevice = devices[0];

    free(devices);
    if (e->physicalDevice == VK_NULL_HANDLE) {
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

    if (vkCreateDevice(e->physicalDevice, &createInfo, NULL, &e->device) !=
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
