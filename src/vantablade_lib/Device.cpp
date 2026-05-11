/*
 * Created by gbian on 06/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// clang-format off
// NOLINTBEGIN(*-include-cleaner, *-signed-bitwise, *-easily-swappable-parameters, *-use-anonymous-namespace, *-diagnostic-old-style-cast, *-pro-type-cstyle-cast, *-pro-type-member-init,*-member-init, *-pro-bounds-constant-array-index, *-qualified-auto, *-uppercase-literal-suffix)
// clang-format on
#include "Vantablade/Device.hpp"
#include "Vantablade/VkObjectTypeResolve.hpp"
#include "Vantablade/VulkanLogInfoCallback.hpp"

DISABLE_WARNINGS_PUSH(4100 4127 4189 4201 4324 4505 4820 26812)
#include <vma/vk_mem_alloc.h>
DISABLE_WARNINGS_POP()

template <typename Fn> Fn Device::loadInstanceProc(VkInstance inst, const char *name) noexcept {
    // NOLINTNEXTLINE(*-pro-type-reinterpret-cast)
    return reinterpret_cast<Fn>(vkGetInstanceProcAddr(inst, name));
}

void Device::loadDebugUtilsFunctions() noexcept {
    if(!enableValidationLayers) { return; }
    const vnd::AutoTimer t{"load Debug Utils Functions"};

    debugFuncs.setObjectName = loadInstanceProc<PFN_vkSetDebugUtilsObjectNameEXT>(instance, "vkSetDebugUtilsObjectNameEXT");
    debugFuncs.cmdBeginLabel = loadInstanceProc<PFN_vkCmdBeginDebugUtilsLabelEXT>(instance, "vkCmdBeginDebugUtilsLabelEXT");
    debugFuncs.cmdEndLabel = loadInstanceProc<PFN_vkCmdEndDebugUtilsLabelEXT>(instance, "vkCmdEndDebugUtilsLabelEXT");
    debugFuncs.cmdInsertLabel = loadInstanceProc<PFN_vkCmdInsertDebugUtilsLabelEXT>(instance, "vkCmdInsertDebugUtilsLabelEXT");
    debugFuncs.queueBeginLabel = loadInstanceProc<PFN_vkQueueBeginDebugUtilsLabelEXT>(instance, "vkQueueBeginDebugUtilsLabelEXT");
    debugFuncs.queueEndLabel = loadInstanceProc<PFN_vkQueueEndDebugUtilsLabelEXT>(instance, "vkQueueEndDebugUtilsLabelEXT");
    debugFuncs.queueInsertLabel = loadInstanceProc<PFN_vkQueueInsertDebugUtilsLabelEXT>(instance, "vkQueueInsertDebugUtilsLabelEXT");

    LINFO("setObjectName   : {}", debugFuncs.setObjectName != nullptr);
    LINFO("cmdBeginLabel   : {}", debugFuncs.cmdBeginLabel != nullptr);
    LINFO("cmdEndLabel     : {}", debugFuncs.cmdEndLabel != nullptr);
    LINFO("cmdInsertLabel  : {}", debugFuncs.cmdInsertLabel != nullptr);
    LINFO("queueBeginLabel : {}", debugFuncs.queueBeginLabel != nullptr);
    LINFO("queueEndLabel   : {}", debugFuncs.queueEndLabel != nullptr);
    LINFO("queueInsertLabel: {}", debugFuncs.queueInsertLabel != nullptr);
}

// ---------------------------------------------------------------------------
// Helper interno: costruisce VkDebugUtilsLabelEXT senza heap allocation.
// Usato da tutti i wrapper che richiedono un colore.
// color è std::span<const float, 4>: extent statico garantito a compile time,
// zero-cost, accetta std::array, C-array e DebugLabelColor senza conversione.
// ---------------------------------------------------------------------------
[[nodiscard]] static VkDebugUtilsLabelEXT makeLabel(const char *name, std::span<const float, 4> color) noexcept {
    return VkDebugUtilsLabelEXT{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
        .pNext = nullptr,
        .pLabelName = name,
        .color = {color[0], color[1], color[2], color[3]},
    };
}

void Device::pcmdBeginLabel(VkCommandBuffer cb, const char *name, std::span<const float, 4> color) noexcept {
    if(!enableValidationLayers || debugFuncs.cmdBeginLabel == nullptr) { return; }
    const auto label = makeLabel(name, color);
    debugFuncs.cmdBeginLabel(cb, &label);
    LINFO("Began command buffer label: \"{}\" with color RGBA({:.2f}, {:.2f}, {:.2f}, {:.2f})", name, color[0], color[1], color[2],
          color[3]);
}

void Device::pcmdEndLabel(VkCommandBuffer cb) noexcept {
    if(!enableValidationLayers || debugFuncs.cmdEndLabel == nullptr) { return; }
    debugFuncs.cmdEndLabel(cb);
    LINFO("Ended command buffer label");
}

void Device::pcmdInsertLabel(VkCommandBuffer cb, const char *name, std::span<const float, 4> color) noexcept {
    if(!enableValidationLayers || debugFuncs.cmdInsertLabel == nullptr) { return; }
    const auto label = makeLabel(name, color);
    debugFuncs.cmdInsertLabel(cb, &label);
    LINFO("Inserted command buffer label: \"{}\" with color RGBA({:.2f}, {:.2f}, {:.2f}, {:.2f})", name, color[0], color[1], color[2],
          color[3]);
}

void Device::pqueueBeginLabel(VkQueue queue, const char *name, std::span<const float, 4> color) noexcept {
    if(!enableValidationLayers || debugFuncs.queueBeginLabel == nullptr) { return; }
    const auto label = makeLabel(name, color);
    debugFuncs.queueBeginLabel(queue, &label);
    LINFO("Began queue label: \"{}\" with color RGBA({:.2f}, {:.2f}, {:.2f}, {:.2f})", name, color[0], color[1], color[2], color[3]);
}

void Device::pqueueEndLabel(VkQueue queue) noexcept {
    if(!enableValidationLayers || debugFuncs.queueEndLabel == nullptr) { return; }
    debugFuncs.queueEndLabel(queue);
    LINFO("Ended queue label");
}

void Device::pqueueInsertLabel(VkQueue queue, const char *name, std::span<const float, 4> color) noexcept {
    if(!enableValidationLayers || debugFuncs.queueInsertLabel == nullptr) { return; }
    const auto label = makeLabel(name, color);
    debugFuncs.queueInsertLabel(queue, &label);
    LINFO("Inserted queue label: \"{}\" with color RGBA({:.2f}, {:.2f}, {:.2f}, {:.2f})", name, color[0], color[1], color[2], color[3]);
}

// local callback functions
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                    [[maybe_unused]] void *pUserData) {
    const char *id = pCallbackData->pMessageIdName;
    if(id != nullptr && std::string_view{id} == "Loader Message") { return VK_FALSE; }
    const std::string type = VkDebugUtilsMessageTypeFlagsEXTString(messageType);

    // Structured Header
    printMessageWhitSeverity("================================================================================", messageSeverity);
    const auto msg = FORMAT("{}Message ID: {}({}): {}", type, pCallbackData->pMessageIdName ? pCallbackData->pMessageIdName : "Unknown",
                            pCallbackData->messageIdNumber, pCallbackData->pMessage);
    // Primary Message
    printMessageWhitSeverity(msg, messageSeverity);

    // Contextual Technical Details (Objects, Labels)
    logDebugValidationLayerInfo(pCallbackData, messageSeverity);

    printMessageWhitSeverity("================================================================================", messageSeverity);

    return VK_FALSE;
}

// NOLINTBEGIN(*-use-internal-linkage)
VkResult CreateDebugUtilsMessengerEXT(VkInstance instancein, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                      const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger) noexcept {
    // NOLINTNEXTLINE(*-pro-type-reinterpret-cast)
    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instancein, "vkCreateDebugUtilsMessengerEXT"));
    // clang-format off
    if(func != nullptr) [[likely]] {
        return func(instancein, pCreateInfo, pAllocator, pDebugMessenger);
    } else [[unlikely]] {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
    // clang-format on
}

void DestroyDebugUtilsMessengerEXT(VkInstance instancein, VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator) noexcept {
    // NOLINTNEXTLINE(*-pro-type-reinterpret-cast)
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instancein, "vkDestroyDebugUtilsMessengerEXT"));
    if(func != nullptr) [[likely]] { func(instancein, debugMessenger, pAllocator); }
}
// NOLINTEND(*-use-internal-linkage)

// class member functions
Device::Device(Window &window) : window{window} {
    vkAllocatorCallbacks = vkAllocator.getCallbacks();
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createCommandPool();
    createAllocator();
}

Device::~Device() {
    vkAllocator.dumpReport();
#ifndef NDEBUG
    const vnd::AutoTimer timer("Destroying Device");
#endif
    vmaDestroyAllocator(allocator);
    vkDestroyCommandPool(device_, commandPool, &vkAllocatorCallbacks);
    vkDestroyDevice(device_, &vkAllocatorCallbacks);

    if(enableValidationLayers) { DestroyDebugUtilsMessengerEXT(instance, debugMessenger, &vkAllocatorCallbacks); }

    vkDestroySurfaceKHR(instance, surface_, &vkAllocatorCallbacks);
    vkDestroyInstance(instance, &vkAllocatorCallbacks);
}

void Device::createInstance() {
    if(enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "LittleVulkanEngine App";
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = C_UI32T(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
#ifdef __APPLE__
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
#ifdef NDEBUG
    if(enableValidationLayers) [[unlikely]] {
        createInfo.enabledLayerCount = C_UI32T(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = &debugCreateInfo;
    } else [[likely]] {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }
#else
    if(enableValidationLayers) [[likely]] {
        createInfo.enabledLayerCount = C_UI32T(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = &debugCreateInfo;
    } else [[unlikely]] {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }
#endif

    VK_CHECK(vkCreateInstance(&createInfo, &vkAllocatorCallbacks, &instance), "failed to create instance!");

    hasGflwRequiredInstanceExtensions();
}

void Device::pickPhysicalDevice() {
#ifndef NDEBUG
    const vnd::AutoTimer t{"pick Physical Device"};
#endif
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if(deviceCount == 0) { throw std::runtime_error("failed to find GPUs with Vulkan support!"); }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for(const auto &device : devices) {
        if(isDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }
    }

    if(physicalDevice == VK_NULL_HANDLE) { throw std::runtime_error("failed to find a suitable GPU!"); }

    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    LINFO("Dev count: {}", deviceCount);
    printDeviceInfo(physicalDevice, properties);
}

void Device::createLogicalDevice() {
    const QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    const std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

    for(const uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    // might not really be necessary anymore because device specific validation layers
    // have been deprecated
    if(enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    VK_CHECK(vkCreateDevice(physicalDevice, &createInfo, &vkAllocatorCallbacks, &device_), "failed to create logical device!");

    psetObjectName(instance, "Main Instance");
    psetObjectName(device_, "Main Device");
    psetObjectName(physicalDevice, "Main Physical Device");

    vkGetDeviceQueue(device_, indices.graphicsFamily, 0, &graphicsQueue_);
    vkGetDeviceQueue(device_, indices.presentFamily, 0, &presentQueue_);
    psetObjectName(graphicsQueue_, "Graphics Queue");
    psetObjectName(presentQueue_, "Present Queue");
}

void Device::createCommandPool() {
    const QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VK_CHECK(vkCreateCommandPool(device_, &poolInfo, &vkAllocatorCallbacks, &commandPool), "failed to create command pool!");
    psetObjectName(commandPool, "Command Pool");
}

void Device::createAllocator() {
    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_4;
    allocatorCreateInfo.physicalDevice = physicalDevice;
    allocatorCreateInfo.device = device_;
    allocatorCreateInfo.instance = instance;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
    allocatorCreateInfo.pAllocationCallbacks = &vkAllocatorCallbacks;

    VK_CHECK(vmaCreateAllocator(&allocatorCreateInfo, &allocator), "failed to create VMA allocator!");
}

void Device::createSurface() { window.createWindowSurface(instance, &surface_, &vkAllocatorCallbacks); }

bool Device::isDeviceSuitable(VkPhysicalDevice device) const {
    const QueueFamilyIndices indices = findQueueFamilies(device);

    const bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if(extensionsSupported) {
        const SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate && (supportedFeatures.samplerAnisotropy != 0u);
}

void Device::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = static_cast<VkDebugUtilsMessageSeverityFlagsEXT>(VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                                                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                                                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT);
    createInfo.messageType = static_cast<VkDebugUtilsMessageTypeFlagsEXT>(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                                                          VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                                                          VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT);
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;  // Optional
}

void Device::setupDebugMessenger() {
    if(!enableValidationLayers) { return; }
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerCreateInfo(createInfo);
    VK_CHECK(CreateDebugUtilsMessengerEXT(instance, &createInfo, &vkAllocatorCallbacks, &debugMessenger),
             "failed to set up debug messenger!");
    loadDebugUtilsFunctions();
}

bool Device::checkValidationLayerSupport() {
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for(const char *layerName : validationLayers) {
        const bool layerFound = std::ranges::any_of(availableLayers, [layerName](const VkLayerProperties &props) {
            // NOLINTNEXTLINE(*-pro-bounds-array-to-pointer-decay, *-no-array-decay)
            return std::string_view{layerName} == std::string_view{props.layerName};
        });
        if(!layerFound) { return false; }
    }
    return true;
}

// NOLINTNEXTLINE(*-convert-member-functions-to-static)
std::vector<const char *> Device::getRequiredExtensions() const {
#ifndef NDEBUG
    const vnd::AutoTimer timer{"get Required Extensions"};
#endif
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    const std::span<const char *> extSpan(glfwExtensions, glfwExtensionCount);
    std::vector<const char *> extensions(extSpan.begin(), extSpan.end());

    if(enableValidationLayers) {
        extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#ifdef __APPLE__
        extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif
    }

    return extensions;
}

void Device::hasGflwRequiredInstanceExtensions() {
#ifndef NDEBUG
    const vnd::AutoTimer t{"has Gflw Required Instance Extensions"};
#endif
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    std::unordered_set<std::string_view> available;

#if defined(_DEBUG) || defined(DEBUG)
    std::vector<std::string> availableExtensions;
#endif
    available.reserve(extensionCount);
    for(const auto &[extensionName, specVersion] : extensions) {
#if defined(_DEBUG) || defined(DEBUG)
        availableExtensions.emplace_back(FORMAT("{} (v. {})", extensionName, specVersion));
#endif
        available.emplace(extensionName);
    }

    const auto requiredExtensions = getRequiredExtensions();
    if(!std::ranges::all_of(requiredExtensions, [&](const auto &required) { return available.contains(required); })) [[unlikely]] {
        throw std::runtime_error("Missing required GLFW extension");
    }
#ifdef NDEBUG
    LINFO("available extensions count: {}", available.size());
    LINFO("required extensions count: {}", requiredExtensions.size());
#else
    LINFO("available extensions:\n  {}", FMT_JOIN(availableExtensions, "\n  "));
    LINFO("required extensions:\n  {}", FMT_JOIN(requiredExtensions, "\n  "));
#endif
}

bool Device::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::unordered_set<std::string_view> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for(const auto &extension : availableExtensions) {
        // NOLINTNEXTLINE(*-pro-bounds-array-to-pointer-decay, *-no-array-decay)
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice phdevice) const {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(phdevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(phdevice, &queueFamilyCount, queueFamilies.data());

    for(const auto &[i, queueFamily] : std::views::enumerate(queueFamilies)) {
        const auto ci = C_UI32T(i);
        if(queueFamily.queueCount > 0 && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0u) {
            indices.graphicsFamily = ci;
            indices.graphicsFamilyHasValue = true;
        }

        VkBool32 presentSupport = false;  // NOLINT(*-implicit-bool-conversion)
        vkGetPhysicalDeviceSurfaceSupportKHR(phdevice, ci, surface_, &presentSupport);

        // NOLINTNEXTLINE(*-implicit-bool-conversion)
        if(queueFamily.queueCount > 0 && presentSupport) {
            indices.presentFamily = ci;
            indices.presentFamilyHasValue = true;
        }

        if(indices.isComplete()) { break; }
    }

    return indices;
}

SwapChainSupportDetails Device::querySwapChainSupport(VkPhysicalDevice device) const {
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);

    if(formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);

    if(presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, details.presentModes.data());
    }
    return details;
}

VkFormat Device::findSupportedFormat(std::span<const VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const {
    for(const VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        // NOLINTBEGIN(*-branch-clone)
        // clang-format off
        if(tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if(tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
        // clang-format on
        // NOLINTEND(*-branch-clone)
    }
    throw std::runtime_error("failed to find supported format!");
}

uint32_t Device::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags propertiesp) const {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    const std::bitset<32> typeBits(typeFilter);
    for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if(typeBits.test(i) && (memProperties.memoryTypes[i].propertyFlags & propertiesp) == propertiesp) { return i; }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void Device::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags propertiesp, VkBuffer &buffer,
                          VmaAllocation &allocation) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.requiredFlags = propertiesp;
    if((propertiesp & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0u) {
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }

    VK_CHECK(vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr), "failed to create buffer!");
}

VkCommandBuffer Device::beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer{VK_NULL_HANDLE};
    VK_CHECK(vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer), "failed to allocate single-time command buffer!");

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo), "failed to begin single-time command buffer!");
    pcmdBeginLabel(commandBuffer, "Single Time Commands", DebugColors::Cyan);
    return commandBuffer;
}

void Device::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    pcmdEndLabel(commandBuffer);

    VK_CHECK(vkEndCommandBuffer(commandBuffer), "failed to end single-time command buffer!");

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    pqueueInsertLabel(graphicsQueue_, "Transient Upload Submission", DebugColors::Cyan);

    VK_CHECK(vkQueueSubmit(graphicsQueue_, 1, &submitInfo, VK_NULL_HANDLE), "failed to submit single-time command buffer!");

    VK_CHECK(vkQueueWaitIdle(graphicsQueue_), "failed to wait for queue idle!");

    vkFreeCommandBuffers(device_, commandPool, 1, &commandBuffer);
}

void Device::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;  // Optional
    copyRegion.dstOffset = 0;  // Optional
    copyRegion.size = size;
    pcmdInsertLabel(commandBuffer, "Copy Buffer", DebugColors::Green);
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

void Device::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    pcmdInsertLabel(commandBuffer, "Copy Buffer To Image", DebugColors::Yellow);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = layerCount;

    region.imageOffset = {.x = 0, .y = 0, .z = 0};
    region.imageExtent = {.width = width, .height = height, .depth = 1};

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    endSingleTimeCommands(commandBuffer);
}

void Device::createImageWithInfo(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags propertiesp, VkImage &image,
                                 VmaAllocation &allocation) {
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.requiredFlags = propertiesp;

    VK_CHECK(vmaCreateImage(allocator, &imageInfo, &allocInfo, &image, &allocation, nullptr), "failed to create image!");
}

// clang-format off
// NOLINTEND(*-include-cleaner, *-signed-bitwise, *-easily-swappable-parameters, *-use-anonymous-namespace, *-diagnostic-old-style-cast, *-pro-type-cstyle-cast, *-pro-type-member-init,*-member-init, *-pro-bounds-constant-array-index, *-qualified-auto, *-uppercase-literal-suffix)
// clang-format on
