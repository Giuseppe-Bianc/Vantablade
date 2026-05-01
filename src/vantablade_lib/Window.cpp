/*
* Created by gbian on 01/05/2026.
* Copyright (c) 2026 All rights reserved.
*/

#include "Vantablade/Window.hpp"

#include "Vantablade/Monitor.hpp"
#include "Vantablade/vulkanCheck.hpp"

    DISABLE_WARNINGS_PUSH(26432 26447)
    Window::Window(const int w, const int h, const std::string_view &window_name) noexcept : width(w), height(h), windowName(window_name) {
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
        vnd::AutoTimer timer("glfw_window creation");

        // Use std::unique_ptr with custom deleter
        window = glfwCreateWindow(width, height, windowName.data(), nullptr, nullptr);

        if(!window) {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window.");
        }
        glfwSetKeyCallback(window, keyCallback);
    }

    void Window::setHints() const noexcept {
        vnd::AutoTimer timer("set glfw hints");
        // Set GLFW context version and profile
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
        // glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    }

    void Window::initializeGLFW() {
        vnd::AutoTimer timer("glfw setup");
        if(!glfwInit()) {
            LCRITICAL("Failed to initialize GLFW");
            throw std::runtime_error("Failed to initialize GLFW.");
        }
        if(!glfwVulkanSupported()) {
            glfwTerminate();
            LCRITICAL("Failed to initialize GLFW. Vulkan not supported");
            throw std::runtime_error("Failed to initialize GLFW. Vulkan not supported");
        }
        glfwSetErrorCallback(errorCallback);
    }

    void Window::centerWindow() {
        vnd::Timer monitort("get primary Monitor");
        GLFWmonitor *primaryMonitor = glfwGetPrimaryMonitor();
        if(!primaryMonitor) { throw std::runtime_error("Failed to get the primary monitor."); }
        LINFO("{}", monitort);

        vnd::Timer modet("get monitor informatoons");
        const Monitor monitorInfo(primaryMonitor);
        LINFO("{}", modet);

        vnd::Timer crepositiont("calculating for reposition");
        int windowWidth;
        int windowHeight;
        glfwGetWindowSize(window, &windowWidth, &windowHeight);
        auto centerX = CALC_CENTRO(monitorInfo.getWidth(), windowWidth);
        auto centerY = CALC_CENTRO(monitorInfo.getHeight(), windowHeight);
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
        LINFO("Monitor:\"{}\", Phys:{}x{}mm, Scale:({}/{}), Pos:({}/{})", glfwGetMonitorName(primaryMonitor),
              monitorInfo.getPhysicalWidth(), monitorInfo.getPhysicalHeight(), monitorInfo.getScaleX(), monitorInfo.getScaleY(),
              monitorInfo.getXPos(), monitorInfo.getYPos());
        LINFO("Monitor Mode:{}", monitorInfo.formatMode());
        LINFO("Created the window {0}: (w: {1}, h: {2}, pos:({3}/{4}))", windowName.data(), width, height, centerX, centerY);
    }

    void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
        VK_CHECK(glfwCreateWindowSurface(instance, window, nullptr, surface), "failed to craete window surface");
    }

    void Window::framebufferResizeCallback(GLFWwindow *window, int width, int height) noexcept {
        auto lveWindow = static_cast<Window *>(glfwGetWindowUserPointer(window));
        lveWindow->framebufferResized = true;
        lveWindow->width = width;
        lveWindow->height = height;
    }
