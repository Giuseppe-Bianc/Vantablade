/*
* Created by gbian on 02/05/2026.
* Copyright (c) 2026 All rights reserved.
*/

#include "Vantablade/Application.hpp"
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
}

void Application::createInstance() {
    // Verifica la versione Vulkan disponibile prima di tutto.
    // vkEnumerateInstanceVersion è disponibile da Vulkan 1.1 in poi,
    // quindi è sicuro chiamarla senza istanza.
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

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    createInfo.enabledLayerCount = 0;

    LINFO("creating vulkan instance with {} extensions", createInfo.enabledExtensionCount);

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
    vkDestroyInstance(instance, nullptr);
}