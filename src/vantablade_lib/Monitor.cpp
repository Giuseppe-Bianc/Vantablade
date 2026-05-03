/*
* Created by gbian on 01/05/2026.
* Copyright (c) 2026 All rights reserved.
*/

// NOLINTBEGIN(*-include-cleaner, *-uppercase-literal-suffix,*-uppercase-literal-suffix)
#include "Vantablade/Monitor.hpp"

    Monitor::Monitor(GLFWmonitor *monitorin)
      : monitor(monitorin), monitorWidth(0), monitorHeight(0), physicalWidth(0), physicalHeight(0), scaleX(0.0f), scaleY(0.0f), xPos(0),
        yPos(0) {
        if(monitor == nullptr) { throw std::runtime_error("Failed to get the primary monitor."); }
        mode = glfwGetVideoMode(monitor);
        if(mode == nullptr) { throw std::runtime_error("Failed to get video mode."); }
        fetchMonitorInfo();
    }

    void Monitor::fetchMonitorInfo() noexcept {
        monitorWidth = mode->width;
        monitorHeight = mode->height;
        glfwGetMonitorPos(monitor, &xPos, &yPos);
        glfwGetMonitorContentScale(monitor, &scaleX, &scaleY);
        glfwGetMonitorPhysicalSize(monitor, &physicalWidth, &physicalHeight);
    }

    std::string Monitor::formatMode() const {
        return FORMAT("({}x{}, Bits rgb{}{}{}, RR:{}Hz)", mode->width, mode->height, mode->redBits, mode->greenBits, mode->blueBits,
                      mode->refreshRate);
    }


// NOLINTEND(*-include-cleaner, *-uppercase-literal-suffix,*-uppercase-literal-suffix)