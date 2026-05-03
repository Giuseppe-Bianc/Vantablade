/*
* Created by gbian on 02/05/2026.
* Copyright (c) 2026 All rights reserved.
*/
// NOLINTBEGIN(*-include-cleaner,*-convert-member-functions-to-static, *-signed-bitwise)
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
    createInstance();
    setupDebugMessenger();
}

void Application::createInstance() {
    uint32_t instanceVersion = 0;
    if (vkEnumerateInstanceVersion(&instanceVersion) != VK_SUCCESS) {
        throw std::runtime_error("failed to enumerate vulkan instance version");
    }

    LINFO("vulkan instance version available: {}.{}.{}",
          VK_API_VERSION_MAJOR(instanceVersion),
          VK_API_VERSION_MINOR(instanceVersion),
          VK_API_VERSION_PATCH(instanceVersion));
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = Vantablade::cmake::project_name.data();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_4;

    LINFO("setting the application info for the vulkan instance: {}", appInfo.pApplicationName);

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

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

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

void Application::mainLoop() {
     while(!window.shouldClose()) [[likely]] {
         glfwPollEvents();
     }
}
void Application::cleanup() {
    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    vkDestroyInstance(instance, nullptr);
}
void Application::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
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

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}
std::vector<const char*> Application::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    const std::span<const char*> extSpan(glfwExtensions, glfwExtensionCount);
    std::vector<const char*> extensions(extSpan.begin(), extSpan.end());

    if(enableValidationLayers) {
        extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
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

VkBool32 Application::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
                                    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,[[maybe_unused]] void *pUserData) {
    // Determine the message type
    const std::string type = VkDebugUtilsMessageTypeFlagsEXTString(messageType);

    // Format and log the message
    const auto msg = FORMAT("{}Message ID: {}({}): {}", type, pCallbackData->pMessageIdName ? pCallbackData->pMessageIdName : "Unknown",
                            pCallbackData->messageIdNumber, pCallbackData->pMessage);

    printMessageWhitSeverity(msg, messageSeverity);

    logDebugValidationLayerInfo(pCallbackData, messageSeverity);

    return VK_FALSE;
}

// NOLINTEND(*-include-cleaner,*-convert-member-functions-to-static, *-signed-bitwise)