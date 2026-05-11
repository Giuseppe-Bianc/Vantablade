/*
 * Created by gbian on 06/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

#include "Device.hpp"
#include "headers.hpp"

struct PipelineConfigInfo {
    PipelineConfigInfo() = default;
    PipelineConfigInfo(const PipelineConfigInfo &) = delete;
    PipelineConfigInfo &operator=(const PipelineConfigInfo &) = delete;

    VkPipelineViewportStateCreateInfo viewportInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    std::vector<VkDynamicState> dynamicStateEnables;
    VkPipelineDynamicStateCreateInfo dynamicStateInfo;
    VkPipelineLayout pipelineLayout = nullptr;
    VkRenderPass renderPass = nullptr;
    uint32_t subpass = 0;
};

class Pipeline {
public:
    Pipeline(Device &device, const fs::path &vertFilepath, const fs::path &fragFilepath, const PipelineConfigInfo &configInfo);
    ~Pipeline();

    Pipeline(const Pipeline &) = delete;
    Pipeline &operator=(const Pipeline &) = delete;

    void bind(VkCommandBuffer commandBuffer) const;
    static void defaultPipelineConfigInfo(PipelineConfigInfo &configInfo);

private:
    [[nodiscard]] static std::vector<char> readFile(const fs::path &filepath);

    void createGraphicsPipeline(const fs::path &vertFilepath, const fs::path &fragFilepath, const PipelineConfigInfo &configInfo);
    void createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule);

    Device &device_m;
    VkPipeline graphicsPipeline{VK_NULL_HANDLE};
    VkShaderModule vertShaderModule{VK_NULL_HANDLE};
    VkShaderModule fragShaderModule{VK_NULL_HANDLE};
};
