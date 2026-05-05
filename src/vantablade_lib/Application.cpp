/*
* Created by gbian on 02/05/2026.
* Copyright (c) 2026 All rights reserved.
*/
// NOLINTBEGIN(*-include-cleaner,*-convert-member-functions-to-static, *-signed-bitwise, *-uppercase-literal-suffix)
#include "Vantablade/Application.hpp"

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    if (func != nullptr) [[likely]] {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    if (func != nullptr) [[likely]] {
        func(instance, debugMessenger, pAllocator);
    }
}
void Application::run() {
    //initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}
/*void Application::initWindow() {
    window = vnd_move_always(Window(800, 600, "Vulkan GLFW"));
}*/
void Application::initVulkan() {
    const vnd::AutoTimer timer{"init Vulkan"};
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    loadDebugUtilsFunctions();

    vkutil::setVulkanDebugName(device, instance, "MainInstance", pfnSetDebugUtilsObjectName);
    vkutil::setVulkanDebugName(device, physicalDevice, "SelectedGPU", pfnSetDebugUtilsObjectName);
    vkutil::setVulkanDebugName(device, device, "LogicalDevice", pfnSetDebugUtilsObjectName);
    vkutil::setVulkanDebugName(device, graphicsQueue, "GraphicsQueue", pfnSetDebugUtilsObjectName);
}

void Application::createInstance() {
    uint32_t instanceVersion = 0;
    VK_CHECK(vkEnumerateInstanceVersion(&instanceVersion),"failed to enumerate vulkan instance version");

    LINFO("vulkan instance version available: {}.{}.{}",
          VK_API_VERSION_MAJOR(instanceVersion),
          VK_API_VERSION_MINOR(instanceVersion),
          VK_API_VERSION_PATCH(instanceVersion));
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = Vantablade::cmake::project_name.data();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    LINFO("setting the application info for the vulkan instance: {}", appInfo.pApplicationName);

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

#ifdef __APPLE__
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = C_UI32T(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = C_UI32T(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        // NOLINTNEXTLINE(*-redundant-casting)
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    LINFO("creating vulkan instance with {} extensions and {} validation layers", createInfo.enabledExtensionCount, createInfo.enabledLayerCount);

    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &instance), "failed to create instance!");
    LINFO("vulkan instance created successfully");
}

void Application::mainLoop() {
    while(!window.shouldClose()) [[likely]] {
        glfwPollEvents();
    }
}
void Application::cleanup() {
    const vnd::AutoTimer cleanupTimer{"application cleanup"};
    vkDestroyDevice(device, nullptr);
    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
}
void Application::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
    const vnd::AutoTimer timer("populate debug messenger create info");
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = static_cast<VkDebugUtilsMessageSeverityFlagsEXT>(
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
    );
    createInfo.messageType = static_cast<VkDebugUtilsMessageTypeFlagsEXT>(
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
    );
    createInfo.pfnUserCallback = debugCallback;
}
void Application::setupDebugMessenger() {
    if (!enableValidationLayers) { return; }

    const vnd::AutoTimer timer("setup debug messenger");
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    VK_CHECK(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger), "failed to set up debug messenger");
}
std::vector<const char*> Application::getRequiredExtensions() {
    const vnd::AutoTimer timer{"get Required Extensions"};
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    const std::span<const char*> extSpan(glfwExtensions, glfwExtensionCount);
    std::vector<const char*> extensions(extSpan.begin(), extSpan.end());

    if(enableValidationLayers) {
        extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#ifdef __APPLE__
        extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif
    }

    return extensions;
}
bool Application::checkValidationLayerSupport() {
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for(const char* layerName : validationLayers) {
        const bool layerFound = std::ranges::any_of(availableLayers, [layerName](const VkLayerProperties& props) {
            // NOLINTNEXTLINE(*-pro-bounds-array-to-pointer-decay, *-no-array-decay)
            return std::string_view{layerName} == std::string_view{props.layerName};
        });
        if(!layerFound) { return false; }
    }
    return true;
}

/**
 * @brief Vulkan Debug Messenger Callback.
 *
 * This function is the primary feedback mechanism for the Vulkan Validation Layers.
 * It translates raw Vulkan events into human-readable diagnostic information, adhering
 * to the official Vulkan specification (VK_EXT_debug_utils).
 *
 * ### Technical Aspects of the Callback:
 *
 * 1. **Validation Layers (Spec Enforcement):**
 *    Validation layers act as a runtime interceptor for Vulkan API calls. They verify that
 *    the application state conforms to the "Valid Usage" (VUID) requirements of the spec.
 *    Any violation is reported here with a unique identifier (e.g., VUID-vkCreateInstance-ppEnabledExtensionNames-01388).
 *
 * 2. **Memory Management & Safety:**
 *    Vulkan requires explicit memory management. Errors reported under `VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT`
 *    often relate to:
 *    - Memory Leaks: Resources not destroyed before `vkDestroyDevice`.
 *    - Illegal Aliasing: Multiple resources bound to the same memory without proper synchronization.
 *    - Allocation Flags: Incorrect `VkMemoryPropertyFlags` for the intended usage.
 *
 * 3. **Synchronization & Resource Hazards:**
 *    Messages of type `PERFORMANCE` or `VALIDATION` frequently highlight race conditions:
 *    - RAW (Read-After-Write): Reading a buffer before a previous write has completed.
 *    - WAW (Write-After-Write): Multiple writes to the same resource without a barrier.
 *    - Interpret these using the reported `VkPipelineStageFlags` and `VkAccessFlags`.
 *
 * 4. **Pipeline & Shader Integrity:**
 *    Validates that the `VkGraphicsPipelineCreateInfo` matches the active `VkRenderPass`
 *    and that shader interface variables (layout locations) are consistent across stages.
 *
 * @return VK_FALSE: The application should continue execution after logging.
 *         (Returning VK_TRUE would cause the API call that triggered this callback to fail with VK_ERROR_VALIDATION_FAILED_EXT).
 */
VkBool32 Application::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
                                    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, [[maybe_unused]] void *pUserData) {
    if(std::string_view(pCallbackData->pMessageIdName) == "Loader Message") {
        return VK_FALSE;  // silently ignore loader messages
    }

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
QueueFamilyIndices Application::findQueueFamilies(VkPhysicalDevice phdevice) {
    const vnd::AutoTimer timer("find Queue Families");
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(phdevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(phdevice, &queueFamilyCount, queueFamilies.data());

    for (const auto& [i, queueFamily] : std::views::enumerate(queueFamilies)) {
        const auto ci =C_UI32T(i);
        if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0u) {
            indices.graphicsFamily = ci;
        }

        VkBool32 presentSupport = false; // NOLINT(*-implicit-bool-conversion)
        vkGetPhysicalDeviceSurfaceSupportKHR(phdevice, ci, surface, &presentSupport);

        // NOLINTNEXTLINE(*-implicit-bool-conversion)
        if (presentSupport) {
            indices.presentFamily = ci;
        }

        if (indices.isComplete()) {
            break;
        }
    }

    return indices;
}
bool Application::isDeviceSuitable(VkPhysicalDevice phdevice) {
    const QueueFamilyIndices indices = findQueueFamilies(phdevice);

    return indices.isComplete();
}
void Application::pickPhysicalDevice() {
    const vnd::AutoTimer timer("pick physical device");
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    const auto it = std::ranges::find_if(
        devices,
        [this](const VkPhysicalDevice& dev) { return isDeviceSuitable(dev); }
    );

    if (it == devices.end()) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

    physicalDevice = *it;
}
void Application::createLogicalDevice() {
    const vnd::AutoTimer timer("create logical device");
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    if (!indices.graphicsFamily.has_value()) {
        throw std::runtime_error("No graphics queue family found");
    }

    const auto graphicsFamily = indices.graphicsFamily.value();

    if (!indices.presentFamily.has_value()) {
        throw std::runtime_error("No present queue family found");
    }

    const auto presentFamily = indices.presentFamily.value();

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    const std::set<uint32_t> uniqueQueueFamilies = {
        graphicsFamily,
        presentFamily
    };

    for (const uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.emplace_back(queueCreateInfo);
    }

    const VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = C_UI32T(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = 0;

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = C_UI32T(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    VK_CHECK(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) ,"failed to create logical device!");

    vkGetDeviceQueue(device, graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(device, presentFamily, 0, &presentQueue);
}
void Application::loadDebugUtilsFunctions() {
    pfnSetDebugUtilsObjectName = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT"));  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)

    if (pfnSetDebugUtilsObjectName == nullptr) [[unlikely]] {
        throw std::runtime_error("vkSetDebugUtilsObjectNameEXT not loaded");
    }
}
void Application::createSurface() {
    const vnd::AutoTimer timer("create surface");
    window.createWindowSurface(instance, &surface);
}
// NOLINTEND(*-include-cleaner,*-convert-member-functions-to-static, *-signed-bitwise, *-uppercase-literal-suffix)