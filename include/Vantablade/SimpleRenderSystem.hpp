/*
 * Created by gbian on 22/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

#include "Camera.hpp"
#include "Device.hpp"
#include "GameObject.hpp"
#include "Pipeline.hpp"

class SimpleRenderSystem {
public:
    SimpleRenderSystem(Device &device, VkRenderPass renderPass);
    ~SimpleRenderSystem();

    SimpleRenderSystem(const SimpleRenderSystem &) = delete;
    SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

    void renderGameObjects(VkCommandBuffer commandBuffer, const std::vector<GameObject> &gameObjects, const Camera &camera);

private:
    void createPipelineLayout();
    void createPipeline(VkRenderPass renderPass);

    Device &device_m;

    std::unique_ptr<Pipeline> pipeline_m;
    VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
};
