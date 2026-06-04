/*
 * Created by gbian on 08/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

#include "Device.hpp"

class Model {
public:
    struct Vertex {
        glm::vec3 position{};
        glm::vec3 color{};
        glm::vec3 normal{};
        glm::vec2 uv{};

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
        bool operator==(const Vertex &other) const {
            return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
        }
    };

    struct Builder {
        std::vector<Vertex> vertices{};
        std::vector<uint32_t> indices{};
        void loadModel(const std::string &filename);
    };

    Model(Device &device, const Builder &builder);
    ~Model();

    Model(const Model &) = delete;
    Model &operator=(const Model &) = delete;

    static std::unique_ptr<Model> createModelFromFile(Device &device, const std::string &filepath);

    void bind(VkCommandBuffer commandBuffer) const;
    void draw(VkCommandBuffer commandBuffer) const;

    [[nodiscard]] uint32_t getVertexCount() const noexcept { return vertexCount; }

private:
    void createVertexBuffers(const std::vector<Vertex> &vertices);
    void createIndexBuffers(const std::vector<uint32_t> &indices);

    Device &device_m;
    VkBuffer vertexBuffer{VK_NULL_HANDLE};
    VmaAllocation vertexBufferAllocation{VK_NULL_HANDLE};  // replaces VkDeviceMemory
    uint32_t vertexCount{0};
    bool hasIndexBuffer{false};
    VkBuffer indexBuffer{VK_NULL_HANDLE};
    VmaAllocation indexBufferAllocation{VK_NULL_HANDLE};
    uint32_t indexCount{0};
};