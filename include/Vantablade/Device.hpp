/*
 * Created by gbian on 06/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once
// clang-format off
#include "DeviceInfo.hpp"
#include "Window.hpp"
#include "VulkanAllocator.hpp"
// clang-format on

// Forward declaration for VMA
VK_DEFINE_HANDLE(VmaAllocator)
VK_DEFINE_HANDLE(VmaAllocation)

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
    uint32_t graphicsFamily{0};
    uint32_t presentFamily{0};
    bool graphicsFamilyHasValue{false};
    bool presentFamilyHasValue{false};

    // CONST: pure predicate — no mutation, noexcept guaranteed.
    [[nodiscard]] bool isComplete() const noexcept { return graphicsFamilyHasValue && presentFamilyHasValue; }
};

struct DebugUtilsFunctions {
    PFN_vkSetDebugUtilsObjectNameEXT setObjectName{nullptr};
    PFN_vkCmdBeginDebugUtilsLabelEXT cmdBeginLabel{nullptr};
    PFN_vkCmdEndDebugUtilsLabelEXT cmdEndLabel{nullptr};
    PFN_vkCmdInsertDebugUtilsLabelEXT cmdInsertLabel{nullptr};
    PFN_vkQueueBeginDebugUtilsLabelEXT queueBeginLabel{nullptr};
    PFN_vkQueueEndDebugUtilsLabelEXT queueEndLabel{nullptr};
    PFN_vkQueueInsertDebugUtilsLabelEXT queueInsertLabel{nullptr};
};

using DebugLabelColor = std::array<float, 4>;

// Colori predefiniti come constexpr: zero overhead, disponibili ovunque.
namespace DebugColors {
    static inline constexpr DebugLabelColor Red = {1.0f, 0.0f, 0.0f, 1.0f};
    static inline constexpr DebugLabelColor Green = {0.0f, 1.0f, 0.0f, 1.0f};
    static inline constexpr DebugLabelColor Blue = {0.0f, 0.0f, 1.0f, 1.0f};
    static inline constexpr DebugLabelColor Yellow = {1.0f, 1.0f, 0.0f, 1.0f};
    static inline constexpr DebugLabelColor Cyan = {0.0f, 1.0f, 1.0f, 1.0f};
    static inline constexpr DebugLabelColor White = {1.0f, 1.0f, 1.0f, 1.0f};
    static inline constexpr DebugLabelColor None = {0.0f, 0.0f, 0.0f, 0.0f};
}  // namespace DebugColors

class Device {
public:
    // PERF: compile-time constant promoted from per-instance non-static member.
    // Original 'const bool' member wasted ≥1 byte per instance for a value that
    // is identical for every Device and known at compile time.
#ifdef NDEBUG
    static inline constexpr bool enableValidationLayers = false;
#else
    static inline constexpr bool enableValidationLayers = true;
#endif

    explicit Device(Window &window);
    ~Device();

    // Not copyable or movable
    Device(const Device &) = delete;
    // STYLE: return type corrected from void to Device& — void is non-standard
    // for a deleted copy-assignment operator and surprises readers.
    Device &operator=(const Device &) = delete;
    Device(Device &&) = delete;
    Device &operator=(Device &&) = delete;

    // CONST: all getters marked const — a const Device& can now be passed to
    // consumers without forcing the entire call chain to be non-const.
    [[nodiscard]] VkCommandPool getCommandPool() const noexcept { return commandPool; }
    [[nodiscard]] VkDevice device() const noexcept { return device_; }
    [[nodiscard]] VkSurfaceKHR surface() const noexcept { return surface_; }
    [[nodiscard]] VkQueue graphicsQueue() const noexcept { return graphicsQueue_; }
    [[nodiscard]] VkQueue presentQueue() const noexcept { return presentQueue_; }
    [[nodiscard]] VmaAllocator getAllocator() const noexcept { return allocator; }
    [[nodiscard]] const VkAllocationCallbacks *getVkAllocator() noexcept { return &vkAllocatorCallbacks; }

    // CONST: query methods do not modify *this.
    [[nodiscard]] SwapChainSupportDetails getSwapChainSupport() const { return querySwapChainSupport(physicalDevice); }
    [[nodiscard]] uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags propertiesp) const;
    [[nodiscard]] QueueFamilyIndices findPhysicalQueueFamilies() const { return findQueueFamilies(physicalDevice); }

    // SPAN: std::span<const VkFormat> replaces const std::vector<VkFormat>&.
    // Callers are no longer required to own a vector; a stack array, a span
    // over a contiguous range, or an initializer_list all work without copying.
    [[nodiscard]] VkFormat findSupportedFormat(std::span<const VkFormat> candidates, VkImageTiling tiling,
                                               VkFormatFeatureFlags features) const;

    // Buffer Helper Functions
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags propertiesp, VkBuffer &buffer,
                      VmaAllocation &allocation);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

    void createImageWithInfo(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags propertiesp, VkImage &image,
                             VmaAllocation &allocation);

    VkPhysicalDeviceProperties properties{};

private:
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool();
    void createAllocator();
    template <typename Fn> [[nodiscard]] static Fn loadInstanceProc(VkInstance inst, const char *name) noexcept;

    // Carica tutti i function pointer debug_utils dopo la creazione dell'istanza.
    void loadDebugUtilsFunctions() noexcept;
    template <typename T> void psetObjectName(T handle, const char *name) noexcept;
    // Command buffer labels
    void pcmdBeginLabel(VkCommandBuffer cb, const char *name, std::span<const float, 4> color) noexcept;
    void pcmdEndLabel(VkCommandBuffer cb) noexcept;
    void pcmdInsertLabel(VkCommandBuffer cb, const char *name, std::span<const float, 4> color) noexcept;

    // Queue labels
    void pqueueBeginLabel(VkQueue queue, const char *name, std::span<const float, 4> color) noexcept;
    void pqueueEndLabel(VkQueue queue) noexcept;
    void pqueueInsertLabel(VkQueue queue, const char *name, std::span<const float, 4> color) noexcept;

    // helper functions
    [[nodiscard]] bool isDeviceSuitable(VkPhysicalDevice device) const;
    [[nodiscard]] std::vector<const char *> getRequiredExtensions() const;
    [[nodiscard]] static bool checkValidationLayerSupport();
    [[nodiscard]] QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;
    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
    void hasGflwRequiredInstanceExtensions();
    [[nodiscard]] static bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    [[nodiscard]] SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;

    VkInstance instance{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT debugMessenger{VK_NULL_HANDLE};
    DebugUtilsFunctions debugFuncs{};
    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
    Window &window;
    VkCommandPool commandPool{VK_NULL_HANDLE};

    VkDevice device_{VK_NULL_HANDLE};
    VmaAllocator allocator{VK_NULL_HANDLE};
    vnd::VulkanAllocator vkAllocator;
    VkAllocationCallbacks vkAllocatorCallbacks;
    VkSurfaceKHR surface_{VK_NULL_HANDLE};
    VkQueue graphicsQueue_{VK_NULL_HANDLE};
    VkQueue presentQueue_{VK_NULL_HANDLE};

    static inline constexpr std::array<const char *, 1> validationLayers{"VK_LAYER_KHRONOS_validation"};
    static inline constexpr std::array<const char *, 1> deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    static inline constexpr float queuePriority = 1.0f;
};