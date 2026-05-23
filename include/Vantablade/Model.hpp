/*
 * Created by gbian on 08/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

#include "Device.hpp"

class Model {
public:
    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;

        [[nodiscard]] static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        [[nodiscard]] static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };

    Model(Device &device, const std::vector<Vertex> &vertices);
    ~Model();

    Model(const Model &) = delete;
    Model &operator=(const Model &) = delete;

    void bind(VkCommandBuffer commandBuffer) const;
    void draw(VkCommandBuffer commandBuffer) const;

private:
    void createVertexBuffers(const std::vector<Vertex> &vertices);

    Device &device_m;
    VkBuffer vertexBuffer{VK_NULL_HANDLE};
    VmaAllocation vertexBufferAllocation{VK_NULL_HANDLE};  // replaces VkDeviceMemory
    uint32_t vertexCount{0};
};