/*
 * Created by gbian on 01/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */

// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "WindowCallback.hpp"

class Window {
public:
    // SAFETY: parameter stays std::string_view (zero-copy at call site);
    // internally stored as std::string to own the data and prevent dangling views.
    Window(int w, int h, std::string_view window_name);
    ~Window();

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;

    [[nodiscard]] GLFWwindow *getGLFWWindow() const noexcept { return window; }
    [[nodiscard]] bool shouldClose() const noexcept { return glfwWindowShouldClose(window); }

    // CONST: neither method modifies *this; missing const was a correctness gap.
    [[nodiscard]] bool wasWindowResized() const noexcept { return framebufferResized; }
    void resetWindowResizedFlag() noexcept { framebufferResized = false; }

    void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface, const VkAllocationCallbacks *allocator = nullptr);

    // CONST + [[nodiscard]]: pure query, return value must not be silently dropped.
    [[nodiscard]] VkExtent2D getExtent() const noexcept { return {C_UI32T(width), C_UI32T(height)}; }

    static void initializeGLFW();

private:
    void initWindow();
    void createWindow();
    void setHints() const noexcept;
    void centerWindow();

    static void framebufferResizeCallback(GLFWwindow *window, int width, int height) noexcept;

    int width{0};
    int height{0};
    bool framebufferResized{false};
    std::string windowName;
    GLFWwindow *window{nullptr};
};

// NOLINTEND(*-include-cleaner)