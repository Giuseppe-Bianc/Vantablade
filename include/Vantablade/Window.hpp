/*
* Created by gbian on 01/05/2026.
* Copyright (c) 2026 All rights reserved.
*/

// NOLINTBEGIN(*-include-cleaner)
#pragma once


#include "WindowCallback.hpp"



    // static void framebufferResizeCallback(GLFWwindow *window, int width, int height) noexcept;
    // NOLINT(*-special-member-functions)
    class Window {
    public:
        Window(const int w, const int h, const std::string_view &window_name) noexcept;
        ~Window();

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        [[nodiscard]] GLFWwindow *getGLFWWindow() const noexcept { return window; }
        [[nodiscard]] bool shouldClose() const noexcept { return glfwWindowShouldClose(window); }
        [[nodiscard]] bool wasWindowResized() noexcept { return framebufferResized; }
        void resetWindowResizedFlag() noexcept { framebufferResized = false; }
        // void swapBuffers() const noexcept { glfwSwapBuffers(window); }
        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

        VkExtent2D getExtent() noexcept { return {C_UI32T(width), C_UI32T(height)}; }

        static void initializeGLFW();

    private:
        void initWindow();

        void createWindow();

        void setHints() const noexcept;

        void centerWindow();
        static void framebufferResizeCallback(GLFWwindow *window, int width, int height) noexcept;

        int width;
        int height;
        bool framebufferResized = false;
        std::string_view windowName;
        GLFWwindow *window{nullptr};
    };

// NOLINTEND(*-include-cleaner)