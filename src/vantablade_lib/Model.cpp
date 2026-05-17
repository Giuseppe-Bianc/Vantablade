/*
 * Created by gbian on 08/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner, *-signed-bitwise)
#include "Vantablade/Model.hpp"
#include "Vantablade/vulkanCheck.hpp"
#include <vma/vk_mem_alloc.h>

Model::Model(Device &device, const std::vector<Vertex> &vertices) : device_m{device} { createVertexBuffers(vertices); }

Model::~Model() {
    // Single call destroys both the VkBuffer and its VmaAllocation.
    vmaDestroyBuffer(device_m.getAllocator(), vertexBuffer, vertexBufferAllocation);
}

void Model::createVertexBuffers(const std::vector<Vertex> &vertices) {
    vertexCount = C_UI32T(vertices.size());
    assert(vertexCount >= 3 && "Vertex count must be at least 3");

    const VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
    device_m.createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffer, vertexBufferAllocation);
    device_m.setObjectName(vertexBuffer, "Vertex Buffer");
    VmaAllocationInfo info{};
    vmaGetAllocationInfo(device_m.getAllocator(), vertexBufferAllocation, &info);
    device_m.setObjectName(info.deviceMemory, "Vertex Buffer Memory");
    void *data = nullptr;
    VK_CHECK(vmaMapMemory(device_m.getAllocator(), vertexBufferAllocation, &data), "failed to map vertex buffer memory!");
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vmaUnmapMemory(device_m.getAllocator(), vertexBufferAllocation);
}

void Model::draw(VkCommandBuffer commandBuffer) const { vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0); }

void Model::bind(VkCommandBuffer commandBuffer) const {
    const std::array<VkBuffer, 1> buffers{vertexBuffer};
    const std::array<VkDeviceSize, 1> offsets{0};
    vkCmdBindVertexBuffers(commandBuffer, 0, C_UI32T(buffers.size()), buffers.data(), offsets.data());
}

std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions() {
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions() {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);
    return attributeDescriptions;
}

// NOLINTEND(*-include-cleaner, *-signed-bitwise)