/*
* Created by gbian on 02/05/2026.
* Copyright (c) 2026 All rights reserved.
*/

#pragma once

#include "Window.hpp"
#include "VulkanLogInfoCallback.hpp"

static inline  constexpr std::array<const char*, 1> validationLayers{{
    "VK_LAYER_KHRONOS_validation"
}};

#ifdef NDEBUG
static inline constexpr bool enableValidationLayers = false;
#else
static inline constexpr bool enableValidationLayers = true;
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;

    bool isComplete() const noexcept {
        return graphicsFamily.has_value();
    }
};

class Application {
public:
    void run();
private:
    Window window{800, 600, "Vulkan GLFW"};
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    //void initWindow();

    void initVulkan();

    void createInstance();

    void mainLoop();

    void cleanup();

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    void setupDebugMessenger();

    std::vector<const char*> getRequiredExtensions();

    bool checkValidationLayerSupport();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,[[maybe_unused]] void* pUserData);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);
    void pickPhysicalDevice();
};
