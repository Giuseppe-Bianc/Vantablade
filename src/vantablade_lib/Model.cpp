/*
 * Created by gbian on 08/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner, *-signed-bitwise)
#include "Vantablade/Model.hpp"
#include "Vantablade/Utils.hpp"
#include "Vantablade/vulkanCheck.hpp"
#include <vk_mem_alloc.h>

#if defined(_MSC_VER) && _MSC_VER < 1940
PRAGMA_WARNING_PUSH(3615)
#endif
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#if defined(_MSC_VER) && _MSC_VER < 1940
PRAGMA_WARNING_POP()
#endif

namespace {
    struct VertexHasher {
        std::size_t operator()(Model::Vertex const &vertex) const noexcept {
            std::size_t seed = 0;
            hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
}  // namespace

DISABLE_WARNINGS_PUSH(26432)
void Model::Builder::loadModel(const std::string &filepath) {
    const vnd::AutoTimer timer("loadModel");
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;
    std::string err;

    if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) { throw std::runtime_error(warn + err); }

    const std::size_t estimatedSize = attrib.vertices.size() / 3;
    vertices.clear();
    indices.clear();
    vertices.reserve(estimatedSize);
    indices.reserve(attrib.vertices.size());

    std::unordered_map<Vertex, uint32_t, VertexHasher> uniqueVertices{};
    for(const auto &shape : shapes) {
        for(const auto &[vertex_index, normal_index, texcoord_index] : shape.mesh.indices) {
            Vertex vertex{};

            if(vertex_index >= 0) {
                auto vertex_index3 = C_ST(3 * vertex_index);
                vertex.position = {
                    attrib.vertices[vertex_index3 + 0],
                    attrib.vertices[vertex_index3 + 1],
                    attrib.vertices[vertex_index3 + 2],
                };

                vertex.color = {
                    attrib.colors[vertex_index3 + 0],
                    attrib.colors[vertex_index3 + 1],
                    attrib.colors[vertex_index3 + 2],
                };
            }

            if(normal_index >= 0) {
                auto normal_index3 = C_ST(3 * normal_index);
                vertex.normal = {
                    attrib.normals[normal_index3 + 0],
                    attrib.normals[normal_index3 + 1],
                    attrib.normals[normal_index3 + 2],
                };
            }

            if(texcoord_index >= 0) {
                auto texcoord_index2 = C_ST(2 * texcoord_index);
                vertex.uv = {
                    attrib.texcoords[texcoord_index2 + 0],
                    attrib.texcoords[texcoord_index2 + 1],
                };
            }

            if(!uniqueVertices.contains(vertex)) {
                uniqueVertices[vertex] = C_UI32T(vertices.size());
                vertices.emplace_back(vertex);
            }
            indices.emplace_back(uniqueVertices[vertex]);
        }
    }
}
DISABLE_WARNINGS_POP()

Model::Model(Device &device, const Model::Builder &builder) : device_m{device} {
    VZ_ZONE_SCOPED;
    createVertexBuffers(builder.vertices);
    createIndexBuffers(builder.indices);
}

Model::~Model() {
    VZ_ZONE_SCOPED;
    vmaDestroyBuffer(device_m.getAllocator(), vertexBuffer, vertexBufferAllocation);
    if(hasIndexBuffer) { vmaDestroyBuffer(device_m.getAllocator(), indexBuffer, indexBufferAllocation); }
}

void Model::createVertexBuffers(const std::vector<Vertex> &vertices) {
    VZ_ZONE_SCOPED;
    vertexCount = C_UI32T(vertices.size());
    VZ_PLOT_INT("Total Vertex Count", vertexCount);
    assert(vertexCount >= 3 && "Vertex count must be at least 3");

    const VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

    VkBuffer stagingBuffer{VK_NULL_HANDLE};
    VmaAllocation stagingBufferAllocation{VK_NULL_HANDLE};
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
    VZ_ZONE_SCOPED;
    indexCount = C_UI32T(indices.size());
    hasIndexBuffer = indexCount > 0;

    if(!hasIndexBuffer) { return; }

    const VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;

    VkBuffer stagingBuffer{VK_NULL_HANDLE};
    VmaAllocation stagingBufferAllocation{VK_NULL_HANDLE};
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

std::unique_ptr<Model> Model::createModelFromFile(Device &device, const std::string &filepath) {
    Builder builder{};
    builder.loadModel(filepath);
    return std::make_unique<Model>(device, builder);
}

void Model::draw(VkCommandBuffer commandBuffer) const {
    VZ_ZONE_SCOPED;
    VZ_GPU_ZONE(device_m.getProfiler().getContext(), commandBuffer, "Model::draw");
    if(hasIndexBuffer) {
        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
    } else {
        vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
    }
}

void Model::bind(VkCommandBuffer commandBuffer) const {
    VZ_ZONE_SCOPED;
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
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

    attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
    attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
    attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
    attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});
    return attributeDescriptions;
}

// NOLINTEND(*-include-cleaner, *-signed-bitwise)
