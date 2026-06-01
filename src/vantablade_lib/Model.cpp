/*
 * Created by gbian on 08/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner, *-signed-bitwise)
#include "Vantablade/Model.hpp"
#include "Vantablade/vulkanCheck.hpp"
#include <vk_mem_alloc.h>

Model::Model(Device &device, const Model::Builder &builder) : device_m{device} {
    VZ_ZONE_SCOPED_NAMED("Model::Constructor");
    createVertexBuffers(builder.vertices);
    createIndexBuffers(builder.indices);
}

Model::~Model() {
    VZ_ZONE_SCOPED_NAMED("Model::Destructor");
    vmaDestroyBuffer(device_m.getAllocator(), vertexBuffer, vertexBufferAllocation);
    if(hasIndexBuffer) { vmaDestroyBuffer(device_m.getAllocator(), indexBuffer, indexBufferAllocation); }
}

void Model::createVertexBuffers(const std::vector<Vertex> &vertices) {
    VZ_ZONE_SCOPED_NAMED("Model::createVertexBuffers");
    vertexCount = C_UI32T(vertices.size());
    VZ_PLOT_INT("Total Vertex Count", vertexCount);
    assert(vertexCount >= 3 && "Vertex count must be at least 3");

    const VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferAllocation;
    device_m.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                          stagingBufferAllocation);

    void *data = nullptr;
    VK_CHECK(vmaMapMemory(device_m.getAllocator(), stagingBufferAllocation, &data), "failed to map vertex staging buffer memory!");
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vmaUnmapMemory(device_m.getAllocator(), stagingBufferAllocation);

    device_m.createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferAllocation);
    device_m.setObjectName(vertexBuffer, "Vertex Buffer");

    device_m.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
    vmaDestroyBuffer(device_m.getAllocator(), stagingBuffer, stagingBufferAllocation);
}

void Model::createIndexBuffers(const std::vector<uint32_t> &indices) {
    VZ_ZONE_SCOPED_NAMED("Model::createIndexBuffers");
    indexCount = C_UI32T(indices.size());
    hasIndexBuffer = indexCount > 0;

    if(!hasIndexBuffer) { return; }

    const VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;

    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferAllocation;
    device_m.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                          stagingBufferAllocation);

    void *data = nullptr;
    VK_CHECK(vmaMapMemory(device_m.getAllocator(), stagingBufferAllocation, &data), "failed to map index staging buffer memory!");
    memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
    vmaUnmapMemory(device_m.getAllocator(), stagingBufferAllocation);

    device_m.createBuffer(bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferAllocation);
    device_m.setObjectName(indexBuffer, "Index Buffer");

    device_m.copyBuffer(stagingBuffer, indexBuffer, bufferSize);
    vmaDestroyBuffer(device_m.getAllocator(), stagingBuffer, stagingBufferAllocation);
}

void Model::draw(VkCommandBuffer commandBuffer) const {
    VZ_ZONE_SCOPED_NAMED("Model::draw");
    VZ_GPU_ZONE(device_m.getProfiler().getContext(), commandBuffer, "Model::draw");
    if(hasIndexBuffer) {
        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
    } else {
        vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
    }
}

void Model::bind(VkCommandBuffer commandBuffer) const {
    VZ_ZONE_SCOPED_NAMED("Model::bind");
    VZ_GPU_ZONE(device_m.getProfiler().getContext(), commandBuffer, "Model::bind");
    const std::array<VkBuffer, 1> buffers{vertexBuffer};
    const std::array<VkDeviceSize, 1> offsets{0};
    vkCmdBindVertexBuffers(commandBuffer, 0, C_UI32T(buffers.size()), buffers.data(), offsets.data());

    if(hasIndexBuffer) { vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32); }
}

std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions() {
    VZ_ZONE_SCOPED;
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions() {
    VZ_ZONE_SCOPED;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);
    return attributeDescriptions;
}

// NOLINTEND(*-include-cleaner, *-signed-bitwise)