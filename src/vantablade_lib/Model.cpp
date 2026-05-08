/*
 * Created by gbian on 08/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#include "Vantablade/Model.hpp"
#include "Vantablade/vulkanCheck.hpp"
#include <vma/vk_mem_alloc.h>

Model::Model(Device &device, const std::vector<Vertex> &vertices) : lveDevice{device} { createVertexBuffers(vertices); }

Model::~Model() {
    // Single call destroys both the VkBuffer and its VmaAllocation.
    vmaDestroyBuffer(lveDevice.getAllocator(), vertexBuffer, vertexBufferAllocation);
}

void Model::createVertexBuffers(const std::vector<Vertex> &vertices) {
    vertexCount = static_cast<uint32_t>(vertices.size());
    assert(vertexCount >= 3 && "Vertex count must be at least 3");

    const VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

    lveDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffer,
                           vertexBufferAllocation);

    void *data = nullptr;
    VK_CHECK(vmaMapMemory(lveDevice.getAllocator(), vertexBufferAllocation, &data), "failed to map vertex buffer memory!");
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vmaUnmapMemory(lveDevice.getAllocator(), vertexBufferAllocation);
}

void Model::draw(VkCommandBuffer commandBuffer) { vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0); }

void Model::bind(VkCommandBuffer commandBuffer) {
    const VkBuffer buffers[] = {vertexBuffer};
    const VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions() {
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions() {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = 0;
    return attributeDescriptions;
}