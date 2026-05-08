/*
 * Created by gbian on 01/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner, *-easily-swappable-parameters, *-convert-member-functions-to-static, *-suspicious-stringview-data-usage)
#include "Vantablade/Window.hpp"

#include "Vantablade/Monitor.hpp"
#include "Vantablade/vulkanCheck.hpp"

DISABLE_WARNINGS_PUSH(26432 26447)
Window::Window(const int w, const int h, const std::string_view window_name) : width(w), height(h), windowName(window_name) {
    initWindow();
}

Window::~Window() {
    glfwDestroyWindow(window);
    glfwTerminate();
}
DISABLE_WARNINGS_POP()

void Window::initWindow() {
    initializeGLFW();
    setHints();
    createWindow();
    centerWindow();
}

void Window::createWindow() {
    const vnd::AutoTimer timer("glfw_window creation");

    window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);

    if(window == nullptr) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window.");
    }
    glfwSetKeyCallback(window, keyCallback);
}

void Window::setHints() const noexcept {
    const vnd::AutoTimer timer("set glfw hints");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    // glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
}

void Window::initializeGLFW() {
    const vnd::AutoTimer timer("glfw setup");
    if(glfwInit() == GLFW_FALSE) {
        LCRITICAL("Failed to initialize GLFW");
        throw std::runtime_error("Failed to initialize GLFW.");
    }
    if(glfwVulkanSupported() == GLFW_FALSE) {
        glfwTerminate();
        LCRITICAL("Failed to initialize GLFW. Vulkan not supported");
        throw std::runtime_error("Failed to initialize GLFW. Vulkan not supported");
    }
    glfwSetErrorCallback(errorCallback);
}

void Window::centerWindow() {
    vnd::Timer monitort("get primary Monitor");
    GLFWmonitor *primaryMonitor = glfwGetPrimaryMonitor();
    if(primaryMonitor == nullptr) { throw std::runtime_error("Failed to get the primary monitor."); }
    LINFO("{}", monitort);

    vnd::Timer modet("get monitor informations");
    const Monitor monitorInfo(primaryMonitor);
    LINFO("{}", modet);

    vnd::Timer crepositiont("calculating for reposition");
    int windowWidth = 0;
    int windowHeight = 0;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    const auto centerX = CALC_CENTRO(monitorInfo.getWidth(), windowWidth);
    const auto centerY = CALC_CENTRO(monitorInfo.getHeight(), windowHeight);
    LINFO("{}", crepositiont);

#ifndef __linux__
    vnd::Timer wrepositiont("window reposition");
    glfwSetWindowPos(window, centerX, centerY);
    int posX = 0;
    int posY = 0;
    glfwGetWindowPos(window, &posX, &posY);
    if(posX != centerX || posY != centerY) { throw std::runtime_error("Failed to position the window at the center."); }
    LINFO("{}", wrepositiont);
#endif

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwShowWindow(window);
    LINFO("Monitor:\"{}\", Phys:{}x{}mm, Scale:({}/{}), Pos:({}/{})", glfwGetMonitorName(primaryMonitor), monitorInfo.getPhysicalWidth(),
          monitorInfo.getPhysicalHeight(), monitorInfo.getScaleX(), monitorInfo.getScaleY(), monitorInfo.getXPos(), monitorInfo.getYPos());
    LINFO("Monitor Mode:{}", monitorInfo.formatMode());
    LINFO("Created the window {}: (w: {}, h: {}, pos:({}/{}))", windowName, width, height, centerX, centerY);
}

void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface, const VkAllocationCallbacks *allocator) {
    VK_CHECK(glfwCreateWindowSurface(instance, window, allocator, surface), "failed to create window surface");
}

void Window::framebufferResizeCallback(GLFWwindow *window, int width, int height) noexcept {
    auto *wwindow = static_cast<Window *>(glfwGetWindowUserPointer(window));
    wwindow->framebufferResized = true;
    wwindow->width = width;
    wwindow->height = height;
}

// NOLINTEND(*-include-cleaner, *-easily-swappable-parameters, *-convert-member-functions-to-static, *-suspicious-stringview-data-usage)