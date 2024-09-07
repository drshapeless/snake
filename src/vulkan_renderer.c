#include "vulkan_renderer.h"

#include "logger.h"
#include "allocator.h"
#include "slstring.h"

#include <SDL3/SDL_vulkan.h>
#include <stdlib.h>

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
              void *pUserData) {
    switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        ERROR("%s", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        WARN("%s", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        INFO("%s", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        TRACE("%s", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
        break;
    }
    DEBUG("validation layer: %s", pCallbackData->pMessage);

    return VK_FALSE;
}

bool initVulkanContext(VulkanContext *context) {
    /* todo: custom allocator */
    context->allocator = NULL;

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_2;
    appInfo.pApplicationName = APPLICATION_NAME;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Shapeless Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    u32 extensionCount = 0;
    const char *const *sdlExtensions =
        SDL_Vulkan_GetInstanceExtensions(&extensionCount);
    const char **extensionNames = (const char **)sdlExtensions;

#ifdef DEBUG
    extensionNames = slAlloc(sizeof(char *) * extensionCount + 1);
    for (u32 i = 0; i < extensionCount; i++) {
        extensionNames[i] = sdlExtensions[i];
    }
    extensionNames[extensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    extensionCount += 1;

    for (u32 i = 0; i < extensionCount; i++) {
        DEBUG("extention name: %s", extensionNames[i]);
    }
#endif

    createInfo.enabledExtensionCount = extensionCount;
    createInfo.ppEnabledExtensionNames = extensionNames;

#ifdef __APPLE__
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    u32 enabledLayerCount = 0;
    const char **enabledLayerNames = NULL;

#ifdef DEBUG
    INFO("Validation layers enabled, enumerating available layers.");
    u32 availableLayerCount = 0;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&availableLayerCount, NULL));
    VkLayerProperties *availableLayers =
        slAlloc(sizeof(VkLayerProperties) * availableLayerCount);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&availableLayerCount,
                                                availableLayers));

    for (u32 i = 0; i < validationLayerCount; i++) {
        INFO("Finding validation layer %s...", validationLayers[i]);
        bool found = false;
        for (u32 j = 0; j < availableLayerCount; j++) {
            if (stringsEqual(validationLayers[i],
                             availableLayers[j].layerName)) {
                INFO("Found %s", validationLayers[i]);
                found = true;
                break;
            }
        }

        if (!found) {
            ERROR("Missing validation layer %s", validationLayers[i]);
            return false;
        }
    }
    INFO("All required validation layers are present.");
    slFree(availableLayers);

    enabledLayerCount = validationLayerCount;
    enabledLayerNames = validationLayers;

#endif

    createInfo.enabledLayerCount = enabledLayerCount;
    createInfo.ppEnabledLayerNames = enabledLayerNames;

#ifdef DEBUG
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    debugCreateInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCreateInfo
        .messageSeverity = /* VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | */
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugCreateInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugCreateInfo.pfnUserCallback = debugCallback;

    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
#else
    createInfo.pNext = NULL;
#endif

    VK_CHECK(
        vkCreateInstance(&createInfo, context->allocator, &context->instance));
    INFO("Vulkan instance created!");

#ifdef DEBUG
    slFree(extensionNames);
#endif

#ifdef DEBUG
    INFO("Creating vulkan debug messager.");
    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            context->instance, "vkCreateDebugUtilsMessengerEXT");
    if (func == NULL) {
        ERROR("failed to create debug messenger!");
        exit(EXIT_FAILURE);
    }

    VK_CHECK(func(context->instance, &debugCreateInfo, context->allocator,
                  &context->debugMessenger));
    DEBUG("debug messenger created.");
#endif

    INFO("Vulkan initialized successfully!");
    return true;
}

void destroyVulkanContext(VulkanContext *context) {
    if (context->debugMessenger != NULL) {
        PFN_vkDestroyDebugUtilsMessengerEXT func =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
                context->instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != NULL) {
            func(context->instance, context->debugMessenger,
                 context->allocator);
        }
    }
    vkDestroyInstance(context->instance, context->allocator);
}
