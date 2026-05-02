/*
* Created by gbian on 02/05/2026.
* Copyright (c) 2026 All rights reserved.
*/

#pragma once

#include "Window.hpp"

class Application {
public:
    void run();
private:
    Window window{800, 600, "Vulkan GLFW"};
    VkInstance instance;

    //void initWindow();

    void initVulkan();

    void createInstance();

    void mainLoop();

    void cleanup();
};
