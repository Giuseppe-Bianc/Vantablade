/*
 * Created by gbian on 08/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#pragma once

#include "Vantablade/headers.hpp"

namespace vnd {

    /**
     * @class VulkanAllocator
     * @brief CPU-side memory tracker for Vulkan allocations.
     *
     * Implements the logic described in Kyle Halladay's "Custom Allocators in Vulkan"
     * to track and log CPU memory usage by the Vulkan driver.
     */
    class VulkanAllocator final {
    public:
        VulkanAllocator() = default;
        ~VulkanAllocator() = default;

        // Not copyable or movable
        VulkanAllocator(const VulkanAllocator &) = delete;
        VulkanAllocator &operator=(const VulkanAllocator &) = delete;
        VulkanAllocator(VulkanAllocator &&) = delete;
        VulkanAllocator &operator=(VulkanAllocator &&) = delete;

        [[nodiscard]] VkAllocationCallbacks getCallbacks() noexcept {
            return VkAllocationCallbacks{.pUserData = this,
                                         .pfnAllocation = &vklAllocation,
                                         .pfnReallocation = &vklReallocation,
                                         .pfnFree = &vklFree,
                                         .pfnInternalAllocation = nullptr,
                                         .pfnInternalFree = nullptr};
        }

        [[nodiscard]] std::size_t getTotalAllocated() const noexcept { return totalAllocated.load(std::memory_order_relaxed); }

    private:
        static VKAPI_ATTR void *VKAPI_CALL vklAllocation(void *pUserData, std::size_t size, std::size_t alignment, VkSystemAllocationScope allocationScope) noexcept;

        static VKAPI_ATTR void *VKAPI_CALL vklReallocation(void *pUserData, void *pOriginal, std::size_t size, std::size_t alignment, VkSystemAllocationScope allocationScope) noexcept;

        static VKAPI_ATTR void VKAPI_CALL vklFree(void *pUserData, void *pMemory) noexcept;

        std::atomic<std::size_t> totalAllocated{0};
    };

}  // namespace vnd