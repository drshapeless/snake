#include "vulkan_engine.h"

#include <stdlib.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3_image/SDL_image.h>

#include "logger.h"
#include "allocator.h"
#include "config.h"
#include "helper.h"
#include "vk_mem_alloc.h"

void createInstance(VkInstance *instance);
void setupDebugMessager(VulkanEngine *engine);
void pickPhysicalDevice(VulkanEngine *engine);
void createLogicalDevice(VulkanEngine *engine);
void createSurface(VulkanEngine *e);
void createSwapChain(VulkanEngine *engine);
void createImageViews(VulkanEngine *engine);
void createRenderPass(VulkanEngine *engine);
void createDescriptorSetLayout(VulkanEngine *engine);
void createGraphicsPipeline(VulkanEngine *engine, const char *vertShader,
                            const char *fragShader);
void createCommandPool(VulkanEngine *engine);
void createCommandBuffers(VulkanEngine *engine);
void createDepthResources(VulkanEngine *engine);
void createFramebuffers(VulkanEngine *engine);
void createTextureImage(VulkanEngine *engine, const char *filename);
void createTextureImageView(VulkanEngine *engine);
void createTextureSampler(VulkanEngine *engine);
void createVertexBuffer(VulkanEngine *engine, Vertex *vertices,
                        u64 verticesSize);
void createIndexBuffer(VulkanEngine *engine, u32 *indices, u64 indicesSize);
void createUniformBuffers(VulkanEngine *engine);
void createDescriptorPool(VulkanEngine *engine);
void createDescriptorSets(VulkanEngine *engine);
void createSyncObjects(VulkanEngine *engine);

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator);

VulkanEngine *createVulkanEngine(SDL_Window *window) {
    VulkanEngine *engine = slAlloc(sizeof(VulkanEngine));
    engine->window = window;
    initVulkan(engine);
    return engine;
}

void initVulkan(VulkanEngine *e) {
    createInstance(&e->instance);
    createSurface(e);
    pickPhysicalDevice(e);
    createLogicalDevice(e);
    createImageViews(e);
    createRenderPass(e);
    createDescriptorSetLayout(e);
    createCommandPool(e);
    createCommandBuffers(e);
    createDepthResources(e);
    createFramebuffers(e);

    /* Must? */
    createTextureImage(e, "texture.jpg");
    createTextureImageView(e);
    createTextureSampler(e);

    /* create vertex buffer */
    /* create index buffer */
    /* create uniform buffer */

    createDescriptorPool(e);
    createDescriptorSets(e);
    createSyncObjects(e);

    createGraphicsPipeline(e, "shader_vert.spv", "shader_frag.spv");
}

void destroyVulkanEngine(VulkanEngine *engine) {
    vkDestroyDevice(engine->device, NULL);
    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(engine->instance, engine->debugMessenger,
                                      NULL);
    }
    vkDestroySurfaceKHR(engine->instance, engine->surface, NULL);
    vkDestroyInstance(engine->instance, NULL);
    slFree(engine);
}

bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    VkLayerProperties *availableLayers =
        slAlloc(sizeof(VkLayerProperties) * layerCount);
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
    const char **extensions = slAlloc(sizeof(char **) * sdlExtensionCount);

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
    appInfo.pApplicationName = APPLICATION_NAME;
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
    slFree(extensions);
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator) {
    PFN_vkDestroyDebugUtilsMessengerEXT func =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL) {
        func(instance, debugMessenger, pAllocator);
    }
}

void setupDebugMessager(VulkanEngine *engine) {
    if (!enableValidationLayers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo = { 0 };
    populateDebugMessengerCreateInfo(&createInfo);

    if (CreateDebugUtilsMessengerEXT(engine->instance, &createInfo, NULL,
                                     &engine->debugMessenger) != VK_SUCCESS) {
        ERROR("failed to set up debug messenger!");
        exit(EXIT_FAILURE);
    }
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device,
                                     VkSurfaceKHR surface) {
    QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

    VkQueueFamilyProperties *queueFamilies =
        malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);

    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             queueFamilies);

    for (int i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
            indices.hasGraphicsFamily = true;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface,
                                             &presentSupport);
        if (presentSupport) {
            indices.presentFamily = i;
            indices.hasPresentFamily = true;
        }

        if (indices.hasGraphicsFamily && indices.hasPresentFamily) {
            break;
        }
    }

    free(queueFamilies);

    return indices;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);

    VkExtensionProperties *availableExtensions =
        malloc(sizeof(VkExtensionProperties) * extensionCount);
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount,
                                         availableExtensions);

    bool support = true;
    for (int i = 0; i < deviceExtensionCount; i++) {
        bool found = false;
        for (int j = 0; j < extensionCount; j++) {
            if (strcmp(availableExtensions[j].extensionName,
                       deviceExtensions[i]) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            support = false;
            break;
        }
    }

    free(availableExtensions);
    return support;
}

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device,
                                              VkSurfaceKHR surface) {
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                              &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, NULL);
    if (formatCount != 0) {
        details.formatCount = formatCount;
        details.formats = malloc(sizeof(VkSurfaceFormatKHR) * formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            device, surface, &details.formatCount, details.formats);
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
                                              &presentModeCount, NULL);

    if (presentModeCount != 0) {
        details.presentModes =
            malloc(sizeof(VkPresentModeKHR) * presentModeCount);
        details.presentModeCount = presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device, surface, &details.presentModeCount, details.presentModes);
    }

    return details;
}

bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices = findQueueFamilies(device, surface);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport =
            querySwapChainSupport(device, surface);
        swapChainAdequate = swapChainSupport.formatCount > 0 &&
                            swapChainSupport.presentModeCount > 0;
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.hasGraphicsFamily && indices.hasPresentFamily &&
           extensionsSupported && swapChainAdequate &&
           supportedFeatures.samplerAnisotropy;
}

void pickPhysicalDevice(VulkanEngine *engine) {
    engine->physicalDevice = VK_NULL_HANDLE;
    u32 deviceCount = 0;
    vkEnumeratePhysicalDevices(engine->instance, &deviceCount, NULL);
    if (deviceCount == 0) {
        ERROR("Failed to find GPUs with Vulkan support!");
        exit(EXIT_FAILURE);
    }

    VkPhysicalDevice *devices = malloc(sizeof(VkPhysicalDevice) * deviceCount);
    vkEnumeratePhysicalDevices(engine->instance, &deviceCount, devices);

    /* loop the device, select with desire feature */
    for (int i = 0; i < deviceCount; i++) {
        if (isDeviceSuitable(devices[i], engine->surface)) {
            engine->physicalDevice = devices[i];
            break;
        }
    }

    if (engine->physicalDevice == VK_NULL_HANDLE) {
        ERROR("Failed to find a suitable GPU!");
        exit(EXIT_FAILURE);
    }
    free(devices);
}

void createLogicalDevice(VulkanEngine *engine) {
    QueueFamilyIndices indices =
        findQueueFamilies(engine->physicalDevice, engine->surface);

    if (indices.graphicsFamily != indices.presentFamily) {
        DEBUG(
            "you are fucked\nfix the queuefamilies for graphics queue and present queue, don't be lazy");
    }

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = deviceExtensionCount;
    createInfo.ppEnabledExtensionNames = deviceExtensions;

    if (vkCreateDevice(engine->physicalDevice, &createInfo, NULL,
                       &engine->device) != VK_SUCCESS) {
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

VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkSurfaceFormatKHR *availableFormats,
                                           u32 availableFormatCount) {
    for (int i = 0; i < availableFormatCount; i++) {
        if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormats[i].colorSpace ==
                VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
            return availableFormats[i];
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(VkPresentModeKHR *availablePresentModes,
                                       u32 availablePresentModeCount) {
    for (int i = 0; i < availablePresentModeCount; i++) {
        if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentModes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR *capabilities,
                            SDL_Window *window) {
    if (capabilities->currentExtent.width != UINT32_MAX) {
        return capabilities->currentExtent;
    } else {
        int width, height;
        SDL_GetWindowSize(window, &width, &height);

        VkExtent2D actualExtent = { (uint32_t)width, (uint32_t)height };
        actualExtent.width = CLAMP(actualExtent.width,
                                   capabilities->maxImageExtent.width,
                                   capabilities->minImageExtent.width);
        actualExtent.height = CLAMP(actualExtent.height,
                                    capabilities->maxImageExtent.height,
                                    capabilities->minImageExtent.height);
        return actualExtent;
    }
}

void createSwapChain(VulkanEngine *engine) {
    SwapChainSupportDetails swapChainSupport =
        querySwapChainSupport(engine->physicalDevice, engine->surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(
        swapChainSupport.formats, swapChainSupport.formatCount);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(
        swapChainSupport.presentModes, swapChainSupport.presentModeCount);
    VkExtent2D extent =
        chooseSwapExtent(&swapChainSupport.capabilities, engine->window);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = engine->surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices =
        findQueueFamilies(engine->physicalDevice, engine->surface);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily,
                                      indices.presentFamily };
    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = NULL; // Optional
    }
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    if (vkCreateSwapchainKHR(engine->device, &createInfo, NULL,
                             &engine->swapChain) != VK_SUCCESS) {
        ERROR("failed to create swap chain!");
        exit(EXIT_FAILURE);
    }

    vkGetSwapchainImagesKHR(engine->device, engine->swapChain, &imageCount,
                            NULL);
    engine->swapChainImages = malloc(sizeof(VkImage) * imageCount);
    engine->swapChainImageCount = imageCount;
    vkGetSwapchainImagesKHR(engine->device, engine->swapChain, &imageCount,
                            engine->swapChainImages);

    engine->swapChainImageFormat = surfaceFormat.format;
    engine->swapChainExtent = extent;
}

VkImageView createImageView(VulkanEngine *engine, VkImage image,
                            VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewInfo = { 0 };
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(engine->device, &viewInfo, NULL, &imageView) !=
        VK_SUCCESS) {
        ERROR("failed to create texture image view!");
        exit(EXIT_FAILURE);
    }

    return imageView;
}

void createImageViews(VulkanEngine *engine) {
    engine->swapChainImageViews =
        malloc(sizeof(VkImageView) * engine->swapChainImageCount);
    engine->swapChainImageViewCount = engine->swapChainImageCount;
    for (int i = 0; i < engine->swapChainImageCount; i++) {
        engine->swapChainImageViews[i] = createImageView(
            engine, engine->swapChainImages[i], engine->swapChainImageFormat,
            VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

VkFormat findSupportedFormat(VulkanEngine *engine, VkFormat *candidates,
                             u32 candidateCount, VkImageTiling tiling,
                             VkFormatFeatureFlags features) {
    for (int i = 0; i < candidateCount; i++) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(engine->physicalDevice,
                                            candidates[i], &props);

        if (tiling == VK_IMAGE_TILING_LINEAR &&
            (props.linearTilingFeatures & features) == features) {
            return candidates[i];
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                   (props.optimalTilingFeatures & features) == features) {
            return candidates[i];
        }
    }

    ERROR("failed to find supported format!");
    return candidates[0];
}

VkFormat findDepthFormat(VulkanEngine *engine) {
    VkFormat formats[] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                           VK_FORMAT_D24_UNORM_S8_UINT };
    return findSupportedFormat(engine, formats,
                               sizeof(formats) / sizeof(VkFormat),
                               VK_IMAGE_TILING_OPTIMAL,
                               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void createRenderPass(VulkanEngine *engine) {
    VkAttachmentDescription colorAttachment = { 0 };
    colorAttachment.format = engine->swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = { 0 };
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment = { 0 };
    depthAttachment.format = findDepthFormat(engine);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = { 0 };
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = { 0 };
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency = { 0 };
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription attachments[] = { colorAttachment,
                                              depthAttachment };
    VkRenderPassCreateInfo renderPassInfo = { 0 };
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount =
        sizeof(attachments) / sizeof(VkAttachmentDescription);
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(engine->device, &renderPassInfo, NULL,
                           &engine->renderPass) != VK_SUCCESS) {
        ERROR("failed to create render pass!");
        exit(EXIT_FAILURE);
    }
}

void createDescriptorSetLayout(VulkanEngine *engine) {
    VkDescriptorSetLayoutBinding uboLayoutBinding = { 0 };
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;

    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = NULL; // Optional

    VkDescriptorSetLayoutBinding samplerLayoutBinding = { 0 };
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = NULL;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding bindings[2] = { uboLayoutBinding,
                                                 samplerLayoutBinding };

    VkDescriptorSetLayoutCreateInfo layoutInfo = { 0 };
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount =
        sizeof(bindings) / sizeof(VkDescriptorSetLayoutBinding);
    layoutInfo.pBindings = bindings;

    if (vkCreateDescriptorSetLayout(engine->device, &layoutInfo, NULL,
                                    &engine->descriptorSetLayout) !=
        VK_SUCCESS) {
        ERROR("failed to create descriptor set layout!\n");
        exit(EXIT_FAILURE);
    }
}

VkShaderModule createShaderModule(VkDevice device, char *code,
                                  unsigned int len) {
    VkShaderModuleCreateInfo createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = len;
    createInfo.pCode = (const u32 *)code;
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, NULL, &shaderModule) !=
        VK_SUCCESS) {
        ERROR("failed to create shader module!");
    }

    return shaderModule;
}

VkVertexInputBindingDescription getVertexInputBindingDescription() {
    VkVertexInputBindingDescription bindingDescription = { 0 };
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

VkVertexInputAttributeDescription *
getVertexInputAttributeDescriptions(uint32_t *len) {
    VkVertexInputAttributeDescription *attributeDescriptions =
        malloc(sizeof(VkVertexInputAttributeDescription) * 3);
    *len = 3;

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    return attributeDescriptions;
}

void createGraphicsPipeline(VulkanEngine *engine, const char *vertShader,
                            const char *fragShader) {
    unsigned int vertShaderLen = 0;
    char *vertShaderCode = readfile(vertShader, &vertShaderLen);
    unsigned int fragShaderLen = 0;
    char *fragShaderCode = readfile(fragShader, &fragShaderLen);

    VkShaderModule vertShaderModule =
        createShaderModule(engine->device, vertShaderCode, vertShaderLen);
    VkShaderModule fragShaderModule =
        createShaderModule(engine->device, fragShaderCode, fragShaderLen);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = { 0 };
    vertShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = { 0 };
    fragShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo,
                                                       fragShaderStageInfo };

    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT,
                                       VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState = { 0 };
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount =
        sizeof(dynamicStates) / sizeof(VkDynamicState);
    dynamicState.pDynamicStates = dynamicStates;

    VkVertexInputBindingDescription bindingDescription =
        getVertexInputBindingDescription();
    uint32_t attributeDescriptionCount = 0;
    VkVertexInputAttributeDescription *attributeDescriptions =
        getVertexInputAttributeDescriptions(&attributeDescriptionCount);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = { 0 };
    vertexInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptionCount;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState = { 0 };
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer = { 0 };
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL; /* polygon mode here!! */
    /* But need to enable gpu feature */
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    VkPipelineMultisampleStateCreateInfo multisampling = { 0 };
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = NULL; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment = { 0 };
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    /* Not needed when blend is disabled */
    /* colorBlendAttachment.blendEnable = VK_TRUE; */
    /* colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; */
    /* colorBlendAttachment.dstColorBlendFactor = */
    /*     VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; */
    /* colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; */
    /* colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; */
    /* colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; */
    /* colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; */

    VkPipelineColorBlendStateCreateInfo colorBlending = { 0 };
    colorBlending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = { 0 };
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &engine->descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = NULL; // Optional

    if (vkCreatePipelineLayout(engine->device, &pipelineLayoutInfo, NULL,
                               &engine->pipelineLayout) != VK_SUCCESS) {
        ERROR("failed to create pipeline layout!");
        exit(EXIT_FAILURE);
    }

    VkPipelineDepthStencilStateCreateInfo depthStencil = { 0 };
    depthStencil.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    /* depthStencil.front = {}; // Optional */
    /* depthStencil.back = {}; // Optional */

    VkGraphicsPipelineCreateInfo pipelineInfo = { 0 };
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = engine->pipelineLayout;
    pipelineInfo.renderPass = engine->renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(engine->device, VK_NULL_HANDLE, 1,
                                  &pipelineInfo, NULL,
                                  &engine->graphicsPipeline) != VK_SUCCESS) {
        ERROR("failed to create graphics pipeline!");
        exit(EXIT_FAILURE);
    }

    free(attributeDescriptions);

    vkDestroyShaderModule(engine->device, fragShaderModule, NULL);
    vkDestroyShaderModule(engine->device, vertShaderModule, NULL);
    free(fragShaderCode);
    free(vertShaderCode);
}

void createCommandPool(VulkanEngine *engine) {
    QueueFamilyIndices queueFamilyIndices =
        findQueueFamilies(engine->physicalDevice, engine->surface);

    VkCommandPoolCreateInfo poolInfo = { 0 };
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

    if (vkCreateCommandPool(engine->device, &poolInfo, NULL,
                            &engine->commandPool) != VK_SUCCESS) {
        ERROR("failed to create command pool!");
        exit(EXIT_FAILURE);
    }
}

void createCommandBuffers(VulkanEngine *engine) {
    engine->commandBuffers =
        malloc(sizeof(VkCommandBuffer) * MAX_FRAMES_IN_FLIGHT);
    engine->commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    VkCommandBufferAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = engine->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = engine->commandBufferCount;

    if (vkAllocateCommandBuffers(engine->device, &allocInfo,
                                 engine->commandBuffers) != VK_SUCCESS) {
        ERROR("failed to allocate command buffers!");
        exit(EXIT_FAILURE);
    }
}

u32 findMemoryType(VkPhysicalDevice physicalDevice, u32 typeFilter,
                   VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (u32 i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) ==
                properties) {
            return i;
        }
    }

    ERROR("failed to find suitable memory type!\n");
    exit(EXIT_FAILURE);
    return 0;
}

void createImage(VulkanEngine *engine, uint32_t width, uint32_t height,
                 VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                 VkMemoryPropertyFlags properties, VkImage *image,
                 VkDeviceMemory *imageMemory) {
    VkImageCreateInfo imageInfo = { 0 };
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0; // Optional
    if (vkCreateImage(engine->device, &imageInfo, NULL, image) != VK_SUCCESS) {
        ERROR("failed to create image!");
        exit(EXIT_FAILURE);
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(engine->device, *image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(
        engine->physicalDevice, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(engine->device, &allocInfo, NULL, imageMemory) !=
        VK_SUCCESS) {
        ERROR("failed to allocate image memory!");
        exit(EXIT_FAILURE);
    }

    vkBindImageMemory(engine->device, *image, *imageMemory, 0);
}

VkCommandBuffer beginSingleTimeCommands(VulkanEngine *engine) {
    VkCommandBufferAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = engine->commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(engine->device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = { 0 };
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void endSingleTimeCommands(VulkanEngine *engine,
                           VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = { 0 };
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(engine->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(engine->graphicsQueue);

    vkFreeCommandBuffers(engine->device, engine->commandPool, 1,
                         &commandBuffer);
}

bool hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
           format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void transitionImageLayout(VulkanEngine *engine, VkImage image, VkFormat format,
                           VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(engine);

    VkImageMemoryBarrier barrier = { 0 };
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (hasStencilComponent(format)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
               newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else {
        ERROR("unsupported layout transition!");
        exit(EXIT_FAILURE);
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0,
                         NULL, 0, NULL, 1, &barrier);

    endSingleTimeCommands(engine, commandBuffer);
}

void createDepthResources(VulkanEngine *engine) {
    VkFormat depthFormat = findDepthFormat(engine);
    createImage(engine, engine->swapChainExtent.width,
                engine->swapChainExtent.height, depthFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &engine->depthImage,
                &engine->depthImageMemory);
    engine->depthImageView = createImageView(
        engine, engine->depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    transitionImageLayout(engine, engine->depthImage, depthFormat,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void createFramebuffers(VulkanEngine *engine) {
    engine->swapChainFramebufferCount = engine->swapChainImageViewCount;
    engine->swapChainFramebuffers =
        malloc(sizeof(VkFramebuffer) * engine->swapChainFramebufferCount);

    VkImageView attachments[] = {};

    for (int i = 0; i < engine->swapChainImageViewCount; i++) {
        VkImageView attachments[] = { engine->swapChainImageViews[i],
                                      engine->depthImageView };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = engine->renderPass;
        framebufferInfo.attachmentCount = ARRAY_LENGTH(attachments);
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = engine->swapChainExtent.width;
        framebufferInfo.height = engine->swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(engine->device, &framebufferInfo, NULL,
                                &engine->swapChainFramebuffers[i]) !=
            VK_SUCCESS) {
            ERROR("failed to create framebuffer!");
            exit(EXIT_FAILURE);
        }
    }
}

void createBuffer(VulkanEngine *engine, VkDeviceSize size,
                  VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                  VkBuffer *buffer, VkDeviceMemory *bufferMemory) {
    VkBufferCreateInfo bufferInfo = { 0 };
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(engine->device, &bufferInfo, NULL, buffer) !=
        VK_SUCCESS) {
        ERROR("failed to create buffer!\n");
        exit(EXIT_FAILURE);
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(engine->device, *buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        findMemoryType(engine->physicalDevice, memRequirements.memoryTypeBits,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(engine->device, &allocInfo, NULL, bufferMemory) !=
        VK_SUCCESS) {
        ERROR("failed to allocate vertex buffer memory!");
        exit(EXIT_FAILURE);
    }

    vkBindBufferMemory(engine->device, *buffer, *bufferMemory, 0);
}

void copyBufferToImage(VulkanEngine *engine, VkBuffer buffer, VkImage image,
                       u32 width, u32 height) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(engine);

    VkBufferImageCopy region = { 0 };
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset.x = 0;
    region.imageOffset.y = 0;
    region.imageOffset.z = 0;
    region.imageExtent.width = width;
    region.imageExtent.height = height;
    region.imageExtent.depth = 1;

    vkCmdCopyBufferToImage(commandBuffer, buffer, image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(engine, commandBuffer);
}

void createTextureImage(VulkanEngine *engine, const char *filename) {
    SDL_Surface *surface = IMG_Load(filename);
    if (surface == NULL) {
        ERROR("failed to load texture image.");
        exit(EXIT_FAILURE);
    }

    SDL_Surface *formattedSurface =
        SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);

    VkDeviceSize imageSize = surface->w * surface->h * 4;
    u32 *pixels = formattedSurface->pixels;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(engine, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 &stagingBuffer, &stagingBufferMemory);

    void *data;
    vkMapMemory(engine->device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, (size_t)imageSize);
    vkUnmapMemory(engine->device, stagingBufferMemory);

    createImage(engine, surface->w, surface->h, VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &engine->textureImage,
                &engine->textureImageMemory);

    transitionImageLayout(engine, engine->textureImage, VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(engine, stagingBuffer, engine->textureImage,
                      (uint32_t)surface->w, (uint32_t)surface->h);
    transitionImageLayout(engine, engine->textureImage, VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(engine->device, stagingBuffer, NULL);
    vkFreeMemory(engine->device, stagingBufferMemory, NULL);

    SDL_DestroySurface(formattedSurface);
    SDL_DestroySurface(surface);
}

void createTextureImageView(VulkanEngine *engine) {
    engine->textureImageView = createImageView(engine, engine->textureImage,
                                               VK_FORMAT_R8G8B8A8_SRGB,
                                               VK_IMAGE_ASPECT_COLOR_BIT);
}

void createTextureSampler(VulkanEngine *engine) {
    VkSamplerCreateInfo samplerInfo = { 0 };
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;

    VkPhysicalDeviceProperties properties = { 0 };
    vkGetPhysicalDeviceProperties(engine->physicalDevice, &properties);
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(engine->device, &samplerInfo, NULL,
                        &engine->textureSampler) != VK_SUCCESS) {
        ERROR("failed to create texture sampler!");
        exit(EXIT_FAILURE);
    }
}

void copyBuffer(VulkanEngine *engine, VkBuffer srcBuffer, VkBuffer dstBuffer,
                VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(engine);

    VkBufferCopy copyRegion = { 0 };
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(engine, commandBuffer);
}

void createVertexBuffer(VulkanEngine *engine, Vertex *vertices,
                        u64 verticesSize) {
    VkDeviceSize bufferSize = verticesSize;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(engine, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 &stagingBuffer, &stagingBufferMemory);

    void *data;
    vkMapMemory(engine->device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, (size_t)bufferSize);
    vkUnmapMemory(engine->device, stagingBufferMemory);

    createBuffer(engine, bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &engine->vertexBuffer,
                 &engine->vertexBufferMemory);

    copyBuffer(engine, stagingBuffer, engine->vertexBuffer, bufferSize);
    vkDestroyBuffer(engine->device, stagingBuffer, NULL);
    vkFreeMemory(engine->device, stagingBufferMemory, NULL);
}

void createIndexBuffer(VulkanEngine *engine, u32 *indices, u64 indicesSize) {
    VkDeviceSize bufferSize = indicesSize;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(engine, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 &stagingBuffer, &stagingBufferMemory);

    void *data;
    vkMapMemory(engine->device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices, (size_t)bufferSize);
    vkUnmapMemory(engine->device, stagingBufferMemory);

    createBuffer(engine, bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &engine->indexBuffer,
                 &engine->indexBufferMemory);

    copyBuffer(engine, stagingBuffer, engine->indexBuffer, bufferSize);

    vkDestroyBuffer(engine->device, stagingBuffer, NULL);
    vkFreeMemory(engine->device, stagingBufferMemory, NULL);
}

void createUniformBuffers(VulkanEngine *engine) {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    engine->uniformBuffers = malloc(sizeof(VkBuffer) * MAX_FRAMES_IN_FLIGHT);
    engine->uniformBufferCount = MAX_FRAMES_IN_FLIGHT;
    engine->uniformBuffersMemory =
        malloc(sizeof(VkDeviceMemory) * MAX_FRAMES_IN_FLIGHT);
    engine->uniformBuffersMemoryCount = MAX_FRAMES_IN_FLIGHT;
    engine->uniformBuffersMapped =
        malloc(sizeof(void *) * MAX_FRAMES_IN_FLIGHT);
    engine->uniformBuffersMappedCount = MAX_FRAMES_IN_FLIGHT;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(engine, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     &engine->uniformBuffers[i],
                     &engine->uniformBuffersMemory[i]);

        vkMapMemory(engine->device, engine->uniformBuffersMemory[i], 0,
                    bufferSize, 0, &engine->uniformBuffersMapped[i]);
    }
}

/* TODO: This needs a big rewrite */
void updateUniformBuffer(VulkanEngine *engine, uint32_t currentImage) {
    /* Uint64 time = SDL_GetTicks() - engine->startTime; */
    /* double timet = time / 400.0f; */

    UniformBufferObject ubo = { 0 };
    glm_mat4_identity(ubo.model);
    glm_rotate(ubo.model, glm_rad(90.0f), (vec3){ 0.0, 0.0, 1.0 });

    glm_lookat((vec3){ 2.0f, 2.0f, 2.0f }, (vec3){ 0.0f, 0.0f, 0.0f },
               (vec3){ 0.0f, 0.0f, 1.0f }, ubo.view);

    glm_perspective(glm_rad(45.0f),
                    (float)engine->swapChainExtent.width /
                        (float)engine->swapChainExtent.height,
                    0.1f, 10.0f, ubo.proj);

    ubo.proj[1][1] *= -1;
    memcpy(engine->uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void createDescriptorPool(VulkanEngine *engine) {
    VkDescriptorPoolSize poolSizes[2] = { 0 };
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = (uint32_t)MAX_FRAMES_IN_FLIGHT;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = (uint32_t)MAX_FRAMES_IN_FLIGHT;

    VkDescriptorPoolCreateInfo poolInfo = { 0 };
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = sizeof(poolSizes) / sizeof(VkDescriptorPoolSize);
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = (uint32_t)MAX_FRAMES_IN_FLIGHT;
    if (vkCreateDescriptorPool(engine->device, &poolInfo, NULL,
                               &engine->descriptorPool) != VK_SUCCESS) {
        ERROR("failed to create descriptor pool!");
        exit(EXIT_FAILURE);
    }
}

void createDescriptorSets(VulkanEngine *engine) {
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        layouts[i] = engine->descriptorSetLayout;
    }

    VkDescriptorSetAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = engine->descriptorPool;
    allocInfo.descriptorSetCount = (uint32_t)MAX_FRAMES_IN_FLIGHT;
    allocInfo.pSetLayouts = layouts;

    engine->descriptorSets =
        malloc(sizeof(VkDescriptorSet) * MAX_FRAMES_IN_FLIGHT);
    engine->descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
    if (vkAllocateDescriptorSets(engine->device, &allocInfo,
                                 engine->descriptorSets) != VK_SUCCESS) {
        ERROR("failed to allocate descriptor sets!");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo = { 0 };
        bufferInfo.buffer = engine->uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = engine->textureImageView;
        imageInfo.sampler = engine->textureSampler;

        VkWriteDescriptorSet descriptorWrites[2] = { 0 };
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = engine->descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;
        descriptorWrites[0].pImageInfo = NULL; // Optional
        descriptorWrites[0].pTexelBufferView = NULL; // Optional

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = engine->descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(engine->device,
                               sizeof(descriptorWrites) /
                                   sizeof(VkWriteDescriptorSet),
                               descriptorWrites, 0, NULL);
    }
}

void createSyncObjects(VulkanEngine *engine) {
    engine->imageAvailableSemaphores =
        malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    engine->imageAvailableSemaphoreCount = MAX_FRAMES_IN_FLIGHT;
    engine->renderFinishedSemaphores =
        malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    engine->renderFinishedSemaphoreCount = MAX_FRAMES_IN_FLIGHT;
    engine->inFlightFences = malloc(sizeof(VkFence) * MAX_FRAMES_IN_FLIGHT);
    engine->inFlightFenceCount = MAX_FRAMES_IN_FLIGHT;

    VkSemaphoreCreateInfo semaphoreInfo = { 0 };
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = { 0 };
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(engine->device, &semaphoreInfo, NULL,
                              &engine->imageAvailableSemaphores[i]) !=
                VK_SUCCESS ||
            vkCreateSemaphore(engine->device, &semaphoreInfo, NULL,
                              &engine->renderFinishedSemaphores[i]) !=
                VK_SUCCESS ||
            vkCreateFence(engine->device, &fenceInfo, NULL,
                          &engine->inFlightFences[i]) != VK_SUCCESS) {
            ERROR("failed to create synchronization objects for a frame!");
            exit(EXIT_FAILURE);
        }
    }
}
