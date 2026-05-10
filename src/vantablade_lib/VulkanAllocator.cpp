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

    /**
     * Performs checked addition of two size_t values.
     *
     * Returns true and writes the result to `result` if the addition does not
     * overflow. Returns false and leaves `result` unmodified if it would overflow.
     *
     * Uses the identity: a + b > SIZE_MAX  <=>  b > SIZE_MAX - a
     * which is safe to evaluate because SIZE_MAX - a never underflows (a <= SIZE_MAX).
     *
     * C++23 does not expose a standard checked-arithmetic API for size_t directly.
     * Compiler builtins (__builtin_add_overflow) are available on GCC/Clang but are
     * not part of the ISO standard. This implementation is fully portable.
     */
    [[nodiscard]] static constexpr bool checked_add(size_t a, size_t b, size_t &result) noexcept {
        if(b > std::numeric_limits<size_t>::max() - a) return false;
        result = a + b;
        return true;
    }

    VKAPI_ATTR void *VKAPI_CALL VulkanAllocator::vklAllocation(void *pUserData, size_t size, size_t alignment,
                                                                VkSystemAllocationScope allocationScope) noexcept {
        if(size == 0) return nullptr;

        auto *const self = static_cast<VulkanAllocator *>(pUserData);

        const size_t effectiveAlign = computeEffectiveAlignment(alignment);

        // Compute the total bytes needed for the malloc block:
        //   sizeof(AllocationHeader)   — space for the header placed immediately before the user pointer
        //   + (effectiveAlign - 1)     — worst-case padding to align the user pointer
        //   + size                     — the user-requested payload
        //
        // Both additions are checked independently to detect unsigned wraparound.
        // The Vulkan spec mandates that `alignment` is a power of two, so effectiveAlign
        // is also a power of two and effectiveAlign - 1 cannot underflow.
        // In practice Vulkan drivers never request host allocations near SIZE_MAX, but
        // correctness must not depend on caller behaviour.
        size_t overhead{};
        if(!checked_add(sizeof(AllocationHeader), effectiveAlign - 1u, overhead)) {
            LERROR("Vulkan CPU allocation overhead overflow. size={}, alignment={}, scope={}", size, alignment,
                   static_cast<int>(allocationScope));
            return nullptr;
        }

        size_t totalSize{};
        if(!checked_add(overhead, size, totalSize)) {
            LERROR("Vulkan CPU allocation totalSize overflow. size={}, alignment={}, scope={}", size, alignment,
                   static_cast<int>(allocationScope));
            return nullptr;
        }

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

    VKAPI_ATTR void *VKAPI_CALL VulkanAllocator::vklReallocation(void *pUserData, void *pOriginal, size_t size, size_t alignment,
                                                                  VkSystemAllocationScope allocationScope) noexcept {
        if(!pOriginal) { return vklAllocation(pUserData, size, alignment, allocationScope); }
        if(size == 0u) {
            vklFree(pUserData, pOriginal);
            return nullptr;
        }

        // Recover original size from the header immediately before the user pointer.
        const auto *const oldHeader = reinterpret_cast<const AllocationHeader *>(reinterpret_cast<uintptr_t>(pOriginal) - sizeof(AllocationHeader));
        const size_t oldSize = oldHeader->size;

        void *const newPtr = vklAllocation(pUserData, size, alignment, allocationScope);
        if(newPtr) {
            // SAFETY: copy only the bytes valid in both old and new buffers.
            std::memcpy(newPtr, pOriginal, std::min(oldSize, size));
            vklFree(pUserData, pOriginal);
        }

        return newPtr;
    }

    VKAPI_ATTR void VKAPI_CALL VulkanAllocator::vklFree(void *pUserData, void *pMemory) noexcept {
        if(!pMemory) { return; }

        auto *const self = static_cast<VulkanAllocator *>(pUserData);

        // Recover header from the sizeof(AllocationHeader) bytes immediately before the user pointer.
        const auto *const header = reinterpret_cast<const AllocationHeader *>(reinterpret_cast<uintptr_t>(pMemory) - sizeof(AllocationHeader));

        self->totalAllocated.fetch_sub(header->size, std::memory_order_relaxed);

        // Free the original malloc base, not the user pointer.
        std::free(header->base);
    }

}  // namespace vnd