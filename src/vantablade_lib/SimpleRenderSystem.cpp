/*
 * Created by gbian on 22/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// clang-format off
// NOLINTBEGIN(*-include-cleaner, *-uppercase-literal-suffix, *-signed-bitwise, *-avoid-magic-numbers, *-magic-numbers, *-pro-type-union-access)
// clang-format on
#include "Vantablade/SimpleRenderSystem.hpp"

#include "Vantablade/vulkanCheck.hpp"

DISABLE_WARNINGS_PUSH(4324)

struct SimplePushConstantData {
    glm::mat4 transform{1.f};
    alignas(16) glm::vec3 color{};
};

DISABLE_WARNINGS_POP()

SimpleRenderSystem::SimpleRenderSystem(Device &device, VkRenderPass renderPass) : device_m{device} {
    createPipelineLayout();
    createPipeline(renderPass);
}

SimpleRenderSystem::~SimpleRenderSystem() { vkDestroyPipelineLayout(device_m.device(), pipelineLayout, nullptr); }

void SimpleRenderSystem::createPipelineLayout() {
#ifndef NDEBUG
    const vnd::AutoTimer timer{"Creating pipeline layout"};
#endif

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);

    const VkPipelineLayoutCreateInfo pipelineLayoutInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                                        .pNext = nullptr,
                                                        .flags = 0,
                                                        .setLayoutCount = 0,
                                                        .pSetLayouts = nullptr,
                                                        .pushConstantRangeCount = 1,
                                                        .pPushConstantRanges = &pushConstantRange};
    VK_CHECK(vkCreatePipelineLayout(device_m.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout), "failed to create pipeline layout!");
    device_m.setObjectName(pipelineLayout, "Main PipelineLayout");
}

void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
#ifndef NDEBUG
    const vnd::AutoTimer timer{"Creating pipeline"};
#endif
    assert(pipelineLayout != VK_NULL_HANDLE && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = pipelineLayout;

    pipeline_m = std::make_unique<Pipeline>(
        device_m, calculateRelativePathToShaders(Vantablade::cmake::project_path(), "simple_shader.vert.opt.spv"),
        calculateRelativePathToShaders(Vantablade::cmake::project_path(), "simple_shader.frag.opt.spv"), pipelineConfig);
}

void SimpleRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject> &gameObjects,  const Camera &camera) {
    pipeline_m->bind(commandBuffer);

    for(auto &obj : gameObjects) {
        obj.transform.rotation.y = glm::mod(obj.transform.rotation.y + 0.01f, glm::two_pi<float>());
        obj.transform.rotation.x = glm::mod(obj.transform.rotation.x + 0.005f, glm::two_pi<float>());

        SimplePushConstantData push{};
        push.color = obj.color;
        push.transform = camera.getProjection() * obj.transform.mat4();

        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                           sizeof(SimplePushConstantData), &push);
        obj.model->bind(commandBuffer);
        obj.model->draw(commandBuffer);
    }
}
// clang-format off
// NOLINTEND(*-include-cleaner, *-uppercase-literal-suffix, *-signed-bitwise, *-avoid-magic-numbers, *-magic-numbers, *-pro-type-union-access)
// clang-format on