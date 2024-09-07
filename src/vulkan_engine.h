#ifndef VULKAN_ENGINE_H
#define VULKAN_ENGINE_H

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <cglm/cglm.h>
#include "defines.h"

typedef struct VulkanEngine {
    SDL_Window *window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;

    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkSurfaceKHR surface;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    VkImage *swapChainImages;
    u32 swapChainImageCount;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    VkImageView *swapChainImageViews;
    u32 swapChainImageViewCount;

    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkFramebuffer *swapChainFramebuffers;
    u32 swapChainFramebufferCount;

    VkCommandPool commandPool;
    VkCommandBuffer *commandBuffers;
    u32 commandBufferCount;

    VkSemaphore *imageAvailableSemaphores;
    u32 imageAvailableSemaphoreCount;
    VkSemaphore *renderFinishedSemaphores;
    u32 renderFinishedSemaphoreCount;
    VkFence *inFlightFences;
    u32 inFlightFenceCount;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    VkBuffer *uniformBuffers;
    uint32_t uniformBufferCount;
    VkDeviceMemory *uniformBuffersMemory;
    uint32_t uniformBuffersMemoryCount;
    void **uniformBuffersMapped;
    uint32_t uniformBuffersMappedCount;

    VkDescriptorPool descriptorPool;
    VkDescriptorSet *descriptorSets;
    uint32_t descriptorSetCount;

    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;

} VulkanEngine;

VulkanEngine *createVulkanEngine(SDL_Window *window);
void destroyVulkanEngine(VulkanEngine *engine);

void initVulkan(VulkanEngine *engine);

typedef struct QueueFamilyIndices {
    u32 graphicsFamily;
    bool hasGraphicsFamily;
    u32 presentFamily;
    bool hasPresentFamily;
} QueueFamilyIndices;

typedef struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR *formats;
    u32 formatCount;
    VkPresentModeKHR *presentModes;
    u32 presentModeCount;
} SwapChainSupportDetails;

typedef struct Vertex {
    vec3 pos;
    vec3 color;
    vec2 texCoord;
} Vertex;

typedef struct UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} UniformBufferObject;

#endif /* VULKAN_ENGINE_H */
