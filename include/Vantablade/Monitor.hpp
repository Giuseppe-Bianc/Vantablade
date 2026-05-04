/*
 * Created by gbian on 01/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

#include "headers.hpp"

class Monitor {
public:
    explicit Monitor(GLFWmonitor *monitorin);
    void fetchMonitorInfo() noexcept;
    [[nodiscard]] std::string formatMode() const;
    [[nodiscard]] int getWidth() const noexcept { return monitorWidth; }
    [[nodiscard]] int getHeight() const noexcept { return monitorHeight; }
    [[nodiscard]] int getPhysicalWidth() const noexcept { return physicalWidth; }
    [[nodiscard]] int getPhysicalHeight() const noexcept { return physicalHeight; }
    [[nodiscard]] float getScaleX() const noexcept { return scaleX; }
    [[nodiscard]] float getScaleY() const noexcept { return scaleY; }
    [[nodiscard]] int getXPos() const noexcept { return xPos; }
    [[nodiscard]] int getYPos() const noexcept { return yPos; }

private:
    GLFWmonitor *monitor{nullptr};     // SAFETY: explicit null — uninitialized raw pointer was indeterminate
    const GLFWvidmode *mode{nullptr};  // SAFETY: same; set by glfwGetVideoMode in constructor body

    // SAFETY: default member initializers — all integral/float members are zero-initialized
    // before the constructor body runs, eliminating indeterminate reads if construction throws.
    int monitorWidth{0};
    int monitorHeight{0};
    int physicalWidth{0};
    int physicalHeight{0};
    float scaleX{0.0f};
    float scaleY{0.0f};
    int xPos{0};
    int yPos{0};
};