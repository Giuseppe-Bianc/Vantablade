/*
 * Created by gbian on 01/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner, *-easily-swappable-parameters, *-convert-member-functions-to-static, *-suspicious-stringview-data-usage)
#include "Vantablade/Window.hpp"

#include "Vantablade/Monitor.hpp"
#include "Vantablade/vulkanCheck.hpp"
#include "Vantablade/Profiler.hpp"

DISABLE_WARNINGS_PUSH(26432 26447)
Window::Window(const int w, const int h, const std::string_view window_name) : width(w), height(h), windowName(window_name) {
    VZ_ZONE_SCOPED;
    initWindow();
}

Window::~Window() {
    VZ_ZONE_SCOPED;
    glfwDestroyWindow(window);
    glfwTerminate();
}
DISABLE_WARNINGS_POP()

void Window::initWindow() {
    VZ_ZONE_SCOPED;
    initializeGLFW();
    setHints();
    createWindow();
    centerWindow();
}

void Window::createWindow() {
    VZ_ZONE_SCOPED;
#ifndef NDEBUG
    const vnd::AutoTimer timer("glfw_window creation");
#endif

    window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);

    if(window == nullptr) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window.");
    }
    glfwSetKeyCallback(window, keyCallback);
}

void Window::setHints() const noexcept {
    VZ_ZONE_SCOPED;
#ifndef NDEBUG
    const vnd::AutoTimer timer("set glfw hints");
#endif
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    // glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
}

void Window::initializeGLFW() {
    VZ_ZONE_SCOPED;
#ifndef NDEBUG
    const vnd::AutoTimer timer("glfw setup");
#endif
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
    VZ_ZONE_SCOPED;
#ifndef NDEBUG
    vnd::Timer monitort("get primary Monitor");
#endif
    GLFWmonitor *primaryMonitor = glfwGetPrimaryMonitor();
    if(primaryMonitor == nullptr) { throw std::runtime_error("Failed to get the primary monitor."); }
#ifndef NDEBUG
    LINFO("{}", monitort);

    vnd::Timer modet("get monitor informations");
#endif
    const Monitor monitorInfo(primaryMonitor);
#ifndef NDEBUG
    LINFO("{}", modet);

    vnd::Timer crepositiont("calculating for reposition");
#endif
    int windowWidth = 0;
    int windowHeight = 0;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    const auto centerX = CALC_CENTRO(monitorInfo.getWidth(), windowWidth);
    const auto centerY = CALC_CENTRO(monitorInfo.getHeight(), windowHeight);
#ifndef NDEBUG
    LINFO("{}", crepositiont);
#endif

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
    VZ_ZONE_SCOPED;
    VK_CHECK(glfwCreateWindowSurface(instance, window, allocator, surface), "failed to create window surface");
}

void Window::framebufferResizeCallback(GLFWwindow *window, int width, int height) noexcept {
    VZ_ZONE_SCOPED;
    auto *wwindow = static_cast<Window *>(glfwGetWindowUserPointer(window));
    wwindow->framebufferResized = true;
    wwindow->width = width;
    wwindow->height = height;
}

// NOLINTEND(*-include-cleaner, *-easily-swappable-parameters, *-convert-member-functions-to-static, *-suspicious-stringview-data-usage)