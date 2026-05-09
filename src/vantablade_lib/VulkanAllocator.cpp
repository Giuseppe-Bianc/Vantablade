/*
 * Created by gbian on 08/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#include "Vantablade/VulkanAllocator.hpp"

namespace vnd {

    struct AllocationHeader final {
        std::size_t size;  // user-requested size, used to update the atomic counter on free
        void *base;        // original std::malloc base; recovered on free to release the block
    };

    [[nodiscard]] static constexpr size_t computeEffectiveAlignment(size_t alignment) noexcept {
        return std::max(alignment, alignof(AllocationHeader));
    }

    VKAPI_ATTR void *VKAPI_CALL VulkanAllocator::allocation(void *pUserData, size_t size, size_t alignment,
                                                            VkSystemAllocationScope allocationScope) noexcept {
        if(size == 0) return nullptr;

        auto *const self = static_cast<VulkanAllocator *>(pUserData);

        const size_t effectiveAlign = computeEffectiveAlignment(alignment);

        const size_t totalSize = sizeof(AllocationHeader) + (effectiveAlign - 1) + size;

        void *const base = std::malloc(totalSize);
        if(!base) {
            LERROR("Vulkan CPU allocation failed. size={}, alignment={}, scope={}", size, alignment, static_cast<int>(allocationScope));
            return nullptr;
        }

        // Find the first address at or after (base + sizeof(AllocationHeader)) that is
        // effectiveAlign-aligned. Since sizeof(AllocationHeader) is a multiple of
        // alignof(AllocationHeader), and effectiveAlign is a multiple of alignof(AllocationHeader),
        // (aligned - sizeof(AllocationHeader)) is also alignof(AllocationHeader)-aligned.
        const uintptr_t addr = reinterpret_cast<uintptr_t>(base) + sizeof(AllocationHeader);
        const uintptr_t aligned = (addr + effectiveAlign - 1u) & ~(effectiveAlign - 1u);

        // Place header in the sizeof(AllocationHeader) bytes immediately before the user pointer.
        // This region is always valid: aligned >= addr = base + sizeof(header), so
        // aligned - sizeof(header) >= base, which is within the allocated block.
        auto *const header = reinterpret_cast<AllocationHeader *>(aligned - sizeof(AllocationHeader));
        header->size = size;
        header->base = base;

        self->totalAllocated.fetch_add(size, std::memory_order_relaxed);
        return reinterpret_cast<void *>(aligned);
    }

    VKAPI_ATTR void *VKAPI_CALL VulkanAllocator::reallocation(void *pUserData, void *pOriginal, size_t size, size_t alignment,
                                                              VkSystemAllocationScope allocationScope) noexcept {
        if(!pOriginal) { return allocation(pUserData, size, alignment, allocationScope); }
        if(size == 0u) {
            free(pUserData, pOriginal);
            return nullptr;
        }

        // Recover original size from the header immediately before the user pointer.
        const auto *const oldHeader = reinterpret_cast<const AllocationHeader *>(reinterpret_cast<uintptr_t>(pOriginal) - sizeof(AllocationHeader));
        const size_t oldSize = oldHeader->size;

        void *const newPtr = allocation(pUserData, size, alignment, allocationScope);
        if(newPtr) {
            // SAFETY: copy only the bytes valid in both old and new buffers.
            std::memcpy(newPtr, pOriginal, std::min(oldSize, size));
            free(pUserData, pOriginal);
        }

        return newPtr;
    }

    VKAPI_ATTR void VKAPI_CALL VulkanAllocator::free(void *pUserData, void *pMemory) noexcept {
        if(!pMemory) { return; }

        auto *const self = static_cast<VulkanAllocator *>(pUserData);

        // Recover header from the sizeof(AllocationHeader) bytes immediately before the user pointer.
        const auto *const header = reinterpret_cast<const AllocationHeader *>(reinterpret_cast<uintptr_t>(pMemory) - sizeof(AllocationHeader));

        self->totalAllocated.fetch_sub(header->size, std::memory_order_relaxed);

        // Free the original malloc base, not the user pointer..
        std::free(header->base);
    }

}  // namespace vnd