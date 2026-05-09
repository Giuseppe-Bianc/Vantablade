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

        /**
         * @brief Returns a VkAllocationCallbacks structure populated with this class's methods.
         */
        [[nodiscard]] VkAllocationCallbacks getCallbacks() noexcept {
            return {.pUserData = this,
                    .pfnAllocation = &allocation,
                    .pfnReallocation = &reallocation,
                    .pfnFree = &free,
                    .pfnInternalAllocation = nullptr,
                    .pfnInternalFree = nullptr};
        }

        [[nodiscard]] size_t getTotalAllocated() const noexcept { return totalAllocated.load(std::memory_order_relaxed); }

    private:
        static VKAPI_ATTR void *VKAPI_CALL allocation(void *pUserData, size_t size, size_t alignment,
                                                      VkSystemAllocationScope allocationScope) noexcept;

        static VKAPI_ATTR void *VKAPI_CALL reallocation(void *pUserData, void *pOriginal, size_t size, size_t alignment,
                                                        VkSystemAllocationScope allocationScope) noexcept;

        static VKAPI_ATTR void VKAPI_CALL free(void *pUserData, void *pMemory) noexcept;

        std::atomic<size_t> totalAllocated{0};
        // Note: In a high-concurrency scenario, a more complex tracker might be needed.
        // For now, atomic counter is sufficient for basic telemetry.
    };

}  // namespace vnd
