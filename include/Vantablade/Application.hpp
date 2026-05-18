/*
 * Created by gbian on 02/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

// clang-format off
#include "Window.hpp"
#include "Device.hpp"
#include "Pipeline.hpp"
#include "SwapChain.hpp"
#include "Model.hpp"
// clang-format on

class Application {
public:
    Application();
    ~Application();

    Application(const Application &) = delete;
    Application &operator=(const Application &) = delete;

    void run();

private:
    void mainLoop();
    void loadModels();
    void createPipelineLayout();
    void createPipeline();
    void createCommandBuffers();
    void freeCommandBuffers();
    void drawFrame();
    void recreateSwapChain();
    void recordCommandBuffer(int imageIndex);

    Window window{wwidth, wheight, wtile};
    Device device_m{window};
    std::unique_ptr<SwapChain> swapChain;
    std::unique_ptr<Pipeline> pipeline;
    VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
    std::vector<VkCommandBuffer> commandBuffers;
    std::unique_ptr<Model> model;
};
