/*
 * Created by gbian on 02/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

// clang-format off
#include "Window.hpp"
#include "Pipeline.hpp"
// clang-format on

static inline constexpr std::array<const char *, 1> validationLayers{{"VK_LAYER_KHRONOS_validation"}};

#ifdef NDEBUG
static inline constexpr bool enableValidationLayers = false;
#else
static inline constexpr bool enableValidationLayers = true;
#endif

static inline constexpr float queuePriority = 1.0f;

/*VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                      const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger);
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator);

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    [[nodiscard]] bool isComplete() const noexcept {  return graphicsFamily.has_value() && presentFamily.has_value(); }
};*/

class Application {
public:
    void run();

private:
    Window window{wwidth, wheight, wtile};
    Device device_m{window};
    VkPipelineLayout pipelineLayout;
    Pipeline pipeline{device_m, "shaders/simple_shader.vert.opt.spv", "shaders/simple_shader.frag.opt.spv",
                      Pipeline::defaultPipelineConfigInfo(wwidth, wheight)};
    /*
        // void initWindow();

        void initVulkan();

        void createInstance();
    */
    void mainLoop();
    /*
        void cleanup();

        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

        void setupDebugMessenger();

        [[nodiscard]] std::vector<const char *> getRequiredExtensions();

        [[nodiscard]] bool checkValidationLayerSupport();

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                            VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                            [[maybe_unused]] void *pUserData);

        [[nodiscard]] QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

        [[nodiscard]] bool isDeviceSuitable(VkPhysicalDevice device);

        void pickPhysicalDevice();

        void createLogicalDevice();

        void loadDebugUtilsFunctions();

        void createSurface();*/
};
