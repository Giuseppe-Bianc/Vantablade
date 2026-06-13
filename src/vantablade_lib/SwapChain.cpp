/*
 * Created by gbian on 07/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner, *-signed-bitwise)
#include "Vantablade/SwapChain.hpp"
#include "Vantablade/vulkanCheck.hpp"

DISABLE_WARNINGS_PUSH(4100 4127 4189 4201 4324 4505 4820 26812)
#include <vk_mem_alloc.h>
DISABLE_WARNINGS_POP()

SwapChain::SwapChain(Device &deviceRef, VkExtent2D extent) : device{deviceRef}, windowExtent{extent} { init(); }

SwapChain::SwapChain(Device &deviceRef, VkExtent2D extent, std::shared_ptr<SwapChain> previous)
  : device{deviceRef}, windowExtent{extent}, oldSwapChain{std::move(previous)} {
    init();
    oldSwapChain = nullptr;
}

void SwapChain::init() {
    createSwapChain();
    createImageViews();
    createRenderPass();
    createDepthResources();
    createFramebuffers();
    createSyncObjects();
}

SwapChain::~SwapChain() {
#ifndef NDEBUG
    const vnd::AutoTimer timer("Destroying SwapChain");
#endif

    auto *const vkdevice = device.device();

    for(auto *const framebuffer : swapChainFramebuffers) { vkDestroyFramebuffer(vkdevice, framebuffer, nullptr); }

    vkDestroyRenderPass(vkdevice, renderPass, nullptr);

    for(auto *const imageView : swapChainImageViews) { vkDestroyImageView(vkdevice, imageView, nullptr); }

    swapChainImageViews.clear();

    if(swapChain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(vkdevice, swapChain, nullptr);
        swapChain = VK_NULL_HANDLE;
    }

    for(const auto &[img, view, alloc] : std::views::zip(depthImages, depthImageViews, depthImageAllocations)) {
        vkDestroyImageView(vkdevice, view, nullptr);
        vmaDestroyImage(device.getAllocator(), img, alloc);
    }

    depthImages.clear();
    depthImageViews.clear();
    depthImageAllocations.clear();

    for(const std::size_t i : std::views::iota(std::size_t{0}, MAX_FRAMES_IN_FLIGHT)) {
        vkDestroySemaphore(vkdevice, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(vkdevice, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(vkdevice, inFlightFences[i], nullptr);
    }
}

VkResult SwapChain::acquireNextImage(uint32_t *imageIndex) {
    auto *const vkdevice = device.device();

    // CONST: waitResult is not reassigned — const makes the intent explicit.
    const VkResult waitResult = vkWaitForFences(vkdevice, 1, &inFlightFences[currentFrame], VK_TRUE, uint64Max);

    if(waitResult != VK_SUCCESS) { return waitResult; }
    return vkAcquireNextImageKHR(vkdevice, swapChain, uint64Max, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, imageIndex);
}

// NOLINTNEXTLINE(*-non-const-parameter)
VkResult SwapChain::submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex) {
    if(imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
        // CONST: result not reassigned after this call.
        VK_CHECK(vkWaitForFences(device.device(), 1, &imagesInFlight[*imageIndex], VK_TRUE, uint64Max),
                 "failed to wait for in-flight image fence!");
    }

    imagesInFlight[*imageIndex] = inFlightFences[currentFrame];

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // SAFETY: std::array<T,N> replaces C-style arrays — size never lost at call boundary.
    // These are all single-element arrays matching the Vulkan spec for this submit pattern.
    const std::array<VkSemaphore, 1> waitSemaphores{imageAvailableSemaphores[currentFrame]};

    const std::array<VkPipelineStageFlags, 1> waitStages{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    submitInfo.waitSemaphoreCount = C_UI32T(waitSemaphores.size());
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages.data();

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = buffers;

    // SAFETY: std::array replaces C-style array for signal semaphores.
    const std::array<VkSemaphore, 1> signalSemaphores{renderFinishedSemaphores[currentFrame]};

    submitInfo.signalSemaphoreCount = C_UI32T(signalSemaphores.size());
    submitInfo.pSignalSemaphores = signalSemaphores.data();

    vkResetFences(device.device(), 1, &inFlightFences[currentFrame]);

    VK_CHECK(vkQueueSubmit(device.graphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]), "failed to submit draw command buffer!");

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = C_UI32T(signalSemaphores.size());
    presentInfo.pWaitSemaphores = signalSemaphores.data();

    // SAFETY: std::array replaces C-style array for swap chains.
    const std::array<VkSwapchainKHR, 1> swapChains{swapChain};

    presentInfo.swapchainCount = C_UI32T(swapChains.size());
    presentInfo.pSwapchains = swapChains.data();
    presentInfo.pImageIndices = imageIndex;

    const VkResult result = vkQueuePresentKHR(device.presentQueue(), &presentInfo);

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return result;
}

void SwapChain::createSwapChain() {
    const SwapChainSupportDetails swapChainSupport = device.getSwapChainSupport();

    const VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    const VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    const VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t mimageCount = swapChainSupport.capabilities.minImageCount + 1;

    if(swapChainSupport.capabilities.maxImageCount > 0 && mimageCount > swapChainSupport.capabilities.maxImageCount) {
        mimageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = device.surface();
    createInfo.minImageCount = mimageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const QueueFamilyIndices indices = device.findPhysicalQueueFamilies();
    if(!indices.graphicsFamily.has_value() || !indices.presentFamily.has_value()) {
        throw std::runtime_error("Incomplete queue family indices");
    }

    // SAFETY: std::array replaces C-style array for queue family indices.
    const std::array<uint32_t, 2> queueFamilyIndices{indices.graphicsFamily.value(), indices.presentFamily.value()};

    if(indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = C_UI32T(queueFamilyIndices.size());
        createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = oldSwapChain == nullptr ? VK_NULL_HANDLE : oldSwapChain->swapChain;

    VK_CHECK(vkCreateSwapchainKHR(device.device(), &createInfo, nullptr, &swapChain), "failed to create swap chain!");

    device.setObjectName(swapChain, "Main SwapChain");

    VK_CHECK(vkGetSwapchainImagesKHR(device.device(), swapChain, &mimageCount, nullptr), "failed to query swapchain image count!");

    swapChainImages.resize(mimageCount);

    VK_CHECK(vkGetSwapchainImagesKHR(device.device(), swapChain, &mimageCount, swapChainImages.data()),
             "failed to retrieve swapchain images!");
    for(const auto &[i, image] : std::views::enumerate(swapChainImages)) {
        const auto name = FORMAT("SwapChain Image[{}]", i);
        device.setObjectName(image, name.c_str());
    }

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void SwapChain::createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());

    for(const auto &[i, image] : std::views::enumerate(swapChainImages)) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = swapChainImageFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VK_CHECK(vkCreateImageView(device.device(), &viewInfo, nullptr, &swapChainImageViews[C_ST(i)]),
                 "failed to create swapchain image view!");
        const auto name = FORMAT("SwapChain ImageView[{}]", i);
        device.setObjectName(swapChainImageViews[C_ST(i)], name.c_str());
    }
}

void SwapChain::createRenderPass() {
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = findDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    const VkAttachmentReference depthAttachmentRef{.attachment = 1, .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = getSwapChainImageFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    const VkAttachmentReference colorAttachmentRef{.attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.dstSubpass = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.srcAccessMask = 0;

    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

    const std::array<VkAttachmentDescription, 2> attachments{colorAttachment, depthAttachment};

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = C_UI32T(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VK_CHECK(vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &renderPass), "failed to create render pass!");

    device.setObjectName(renderPass, "Main RenderPass");
}

void SwapChain::createFramebuffers() {
    swapChainFramebuffers.resize(imageCount());

    for(const auto &[i, imageView] : std::views::enumerate(swapChainImageViews)) {
        const std::array<VkImageView, 2> attachments{imageView, depthImageViews[C_ST(i)]};
        const VkExtent2D extent = getSwapChainExtent();

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = C_UI32T(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        VK_CHECK(vkCreateFramebuffer(device.device(), &framebufferInfo, nullptr, &swapChainFramebuffers[C_ST(i)]),
                 "failed to create framebuffer!");
        const auto name = FORMAT("Framebuffer[{}]", i);
        device.setObjectName(swapChainFramebuffers[C_ST(i)], name.c_str());
    }
}

void SwapChain::createDepthResources() {
    const VkFormat depthFormat = findDepthFormat();
    swapChainDepthFormat = depthFormat;
    const VkExtent2D extent = getSwapChainExtent();
    const std::size_t count = imageCount();

    depthImages.resize(count);
    depthImageAllocations.resize(count);
    depthImageViews.resize(count);

    for(const std::size_t i : std::views::iota(std::size_t{0}, count)) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = extent.width;
        imageInfo.extent.height = extent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = depthFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = 0;

        device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImages[i], depthImageAllocations[i]);
        const auto imgName = FORMAT("Depth Image[{}]", i);
        device.setObjectName(depthImages[i], imgName.c_str());
        VmaAllocationInfo info{};
        vmaGetAllocationInfo(device.getAllocator(), depthImageAllocations[i], &info);
        const auto inf = FORMAT("Depth Image Memory[{}]", i);
        device.setObjectName(info.deviceMemory, inf.c_str());

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = depthImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = depthFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VK_CHECK(vkCreateImageView(device.device(), &viewInfo, nullptr, &depthImageViews[i]), "failed to create texture image view!");
        const auto viewName = FORMAT("Depth ImageView[{}]", i);  // aggiunta
        device.setObjectName(depthImageViews[i], viewName.c_str());
    }
}

void SwapChain::createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.assign(imageCount(), VK_NULL_HANDLE);

    const VkSemaphoreCreateInfo semaphoreInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };
    const VkFenceCreateInfo fenceInfo{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    auto *const vkdevice = device.device();

    for(const std::size_t i : std::views::iota(std::size_t{0}, MAX_FRAMES_IN_FLIGHT)) {
        VK_CHECK(vkCreateSemaphore(vkdevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]),
                 FORMAT("failed to create imageAvailableSemaphores[{}]!", i));
        VK_CHECK(vkCreateSemaphore(vkdevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]),
                 FORMAT("failed to create renderFinishedSemaphores[{}]!", i));
        VK_CHECK(vkCreateFence(vkdevice, &fenceInfo, nullptr, &inFlightFences[i]), FORMAT("failed to create inFlightFences[{}]!", i));

        device.setObjectName(imageAvailableSemaphores[i], FORMAT("ImageAvailable Semaphore[{}]", i).c_str());
        device.setObjectName(renderFinishedSemaphores[i], FORMAT("RenderFinished Semaphore[{}]", i).c_str());
        device.setObjectName(inFlightFences[i], FORMAT("InFlight Fence[{}]", i).c_str());
    }
}

VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    const auto it = std::ranges::find_if(availableFormats, [](const VkSurfaceFormatKHR &availableFormat) {
        return availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    });
    if(it != availableFormats.end()) { return *it; }
    return availableFormats[0];
}

VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
    auto isAvailable = [&](VkPresentModeKHR mode) { return std::ranges::find(availablePresentModes, mode) != availablePresentModes.end(); };

    // Ordine di priorità realistico per rendering interattivo
    if(isAvailable(VK_PRESENT_MODE_MAILBOX_KHR)) {
        LINFO("Present mode: MAILBOX");
        return VK_PRESENT_MODE_MAILBOX_KHR;
    }

    /*if(isAvailable(VK_PRESENT_MODE_FIFO_KHR)) {
        LINFO("Present mode: FIFO (VSync)");
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    if(isAvailable(VK_PRESENT_MODE_FIFO_RELAXED_KHR)) {
        LINFO("Present mode: FIFO_RELAXED");
        return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
    }*/

    if(isAvailable(VK_PRESENT_MODE_IMMEDIATE_KHR)) {
        LINFO("Present mode: IMMEDIATE");
        return VK_PRESENT_MODE_IMMEDIATE_KHR;
    }

    if(isAvailable(VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR)) {
        LINFO("Present mode: SHARED_DEMAND_REFRESH");
        return VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR;
    }

    if(isAvailable(VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR)) {
        LINFO("Present mode: SHARED_CONTINUOUS_REFRESH");
        return VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR;
    }

    // Fallback obbligatorio secondo spec Vulkan
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const {
    if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) { return capabilities.currentExtent; }

    // CONST: actualExtent is modified after construction; width/height are clamped in place.
    VkExtent2D actualExtent = windowExtent;
    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    return actualExtent;
}

VkFormat SwapChain::findDepthFormat() const {
    // SAFETY: std::array replaces C-style array — size preserved at call boundary.
    constexpr std::array<VkFormat, 3> candidates{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    return device.findSupportedFormat(candidates, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

// NOLINTEND(*-include-cleaner, *-signed-bitwise)