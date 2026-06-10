/*
 * Created by gbian on 06/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once
// clang-format off
#include "DeviceInfo.hpp"
#include "Window.hpp"
#include "VkObjectTypeResolve.hpp"
#include "vulkanToString.hpp"
#include "Profiler.hpp"
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
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    [[nodiscard]] bool isComplete() const noexcept { return graphicsFamily.has_value() && presentFamily.has_value(); }
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
    static inline constexpr DebugLabelColor Magenta = {1.0f, 0.0f, 1.0f, 1.0f};
    static inline constexpr DebugLabelColor White = {1.0f, 1.0f, 1.0f, 1.0f};
    static inline constexpr DebugLabelColor None = {0.0f, 0.0f, 0.0f, 0.0f};
}  // namespace DebugColors

// In Device.hpp oppure come tipo interno privato

struct DeviceFeatureChain {
    VkPhysicalDeviceVulkan14Features f14{};
    VkPhysicalDeviceVulkan13Features f13{};
    VkPhysicalDeviceVulkan12Features f12{};
    VkPhysicalDeviceVulkan11Features f11{};
    VkPhysicalDeviceFeatures2 f2{};

    DeviceFeatureChain() noexcept {
        f14.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
        f14.pNext = nullptr;

        f13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        f13.pNext = &f14;

        f12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        f12.pNext = &f13;

        f11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
        f11.pNext = &f12;

        f2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        f2.pNext = &f11;
    }
};

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
    [[nodiscard]] VkInstance getInstance() const noexcept { return instance; }
    [[nodiscard]] VkPhysicalDevice getPhysicalDevice() const noexcept { return physicalDevice; }

    void updateMemoryStats();

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

    template <typename T> void setObjectName(T handle, const char *name) noexcept { psetObjectName(handle, name); }
    // Command buffer labels
    void cmdBeginLabel(VkCommandBuffer cb, const char *name, std::span<const float, 4> color) const noexcept {
        pcmdBeginLabel(cb, name, color);
    }
    void cmdEndLabel(VkCommandBuffer cb) const noexcept { pcmdEndLabel(cb); }
    void cmdInsertLabel(VkCommandBuffer cb, const char *name, std::span<const float, 4> color) const noexcept {
        pcmdInsertLabel(cb, name, color);
    }

    // Queue labels
    void queueBeginLabel(VkQueue queue, const char *name, std::span<const float, 4> color) const noexcept {
        pqueueBeginLabel(queue, name, color);
    }
    void queueEndLabel(VkQueue queue) const noexcept { pqueueEndLabel(queue); }
    void queueInsertLabel(VkQueue queue, const char *name, std::span<const float, 4> color) const noexcept {
        pqueueInsertLabel(queue, name, color);
    }

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
    void pcmdBeginLabel(VkCommandBuffer cb, const char *name, std::span<const float, 4> color) const noexcept;
    void pcmdEndLabel(VkCommandBuffer cb) const noexcept;
    void pcmdInsertLabel(VkCommandBuffer cb, const char *name, std::span<const float, 4> color) const noexcept;

    // Queue labels
    void pqueueBeginLabel(VkQueue queue, const char *name, std::span<const float, 4> color) const noexcept;
    void pqueueEndLabel(VkQueue queue) const noexcept;
    void pqueueInsertLabel(VkQueue queue, const char *name, std::span<const float, 4> color) const noexcept;

    // helper functions
    [[nodiscard]] bool isDeviceSuitable(VkPhysicalDevice device) const;
    [[nodiscard]] std::vector<const char *> getRequiredExtensions() const;
    [[nodiscard]] static bool checkValidationLayerSupport();
    [[nodiscard]] QueueFamilyIndices findQueueFamilies(VkPhysicalDevice phdevice) const;
    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
    void hasGflwRequiredInstanceExtensions();
    [[nodiscard]] static bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    [[nodiscard]] SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;

    VkInstance instance{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT debugMessenger{VK_NULL_HANDLE};
    DebugUtilsFunctions debugFuncs{};
    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
    Window &window_m;
    VkCommandPool commandPool{VK_NULL_HANDLE};

    VkDevice device_{VK_NULL_HANDLE};
    VmaAllocator allocator{VK_NULL_HANDLE};
    VkSurfaceKHR surface_{VK_NULL_HANDLE};
    VkQueue graphicsQueue_{VK_NULL_HANDLE};
    VkQueue presentQueue_{VK_NULL_HANDLE};

    static inline constexpr std::array<const char *, 1> validationLayers{"VK_LAYER_KHRONOS_validation"};
    static inline constexpr std::array<const char *, 1> deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    static inline constexpr float queuePriority = 1.0f;
};

template <typename T> inline void Device::psetObjectName(T handle, const char *name) noexcept {
    if constexpr(!enableValidationLayers) { return; }
    if(debugFuncs.setObjectName == nullptr) { return; }
    // NOLINTNEXTLINE(*-pro-type-reinterpret-cast)
    constexpr VkObjectType objectType = vkutil::vulkanObjectType<T>();
    // NOLINTNEXTLINE(*-pro-type-reinterpret-cast, *-no-int-to-ptr)
    // Avoid -Wuseless-cast on some platforms by temporarily suppressing the warning
    DISABLE_GCC_WARNINGS_PUSH("-Wuseless-cast")
    const auto nhandle = reinterpret_cast<std::uintptr_t>(handle);
    DISABLE_GCC_WARNINGS_POP()

    const VkDebugUtilsObjectNameInfoEXT nameInfo{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .pNext = nullptr,
        .objectType = objectType,
        .objectHandle = C_UI64T(nhandle),
        .pObjectName = name,
    };
    debugFuncs.setObjectName(device_, &nameInfo);

    LINFO("Named '{}' -> {} {:#018x}", name, VkObjectString(objectType), nhandle);
}
