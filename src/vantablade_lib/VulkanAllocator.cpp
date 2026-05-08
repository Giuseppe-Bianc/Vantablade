/*
 * Created by gbian on 08/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#include "Vantablade/VulkanAllocator.hpp"

namespace vnd {

// Helper to store size before the allocated pointer
struct AllocationHeader {
    size_t size;
};

VKAPI_ATTR void* VKAPI_CALL VulkanAllocator::allocation(
    void* pUserData,
    size_t size,
    size_t alignment,
    VkSystemAllocationScope allocationScope) noexcept {
    
    if (size == 0) return nullptr;

    auto* self = static_cast<VulkanAllocator*>(pUserData);
    
    // Allocate with space for header
    size_t totalSize = size + sizeof(AllocationHeader);
    void* ptr = nullptr;

#if defined(_MSC_VER) || defined(__MINGW32__)
    ptr = _aligned_malloc(totalSize, alignment);
#else
    if (posix_memalign(&ptr, alignment, totalSize) != 0) ptr = nullptr;
#endif

    if (ptr) {
        auto* header = static_cast<AllocationHeader*>(ptr);
        header->size = size;
        self->totalAllocated.fetch_add(size, std::memory_order_relaxed);
        
        // Return pointer after header
        return static_cast<char*>(ptr) + sizeof(AllocationHeader);
    }

    LERROR("Vulkan CPU Allocation failed! Size: {}, Alignment: {}, Scope: {}", size, alignment, (int)allocationScope);
    return nullptr;
}

VKAPI_ATTR void* VKAPI_CALL VulkanAllocator::reallocation(
    void* pUserData,
    void* pOriginal,
    size_t size,
    size_t alignment,
    VkSystemAllocationScope allocationScope) noexcept {
    
    if (!pOriginal) return allocation(pUserData, size, alignment, allocationScope);
    if (size == 0) {
        free(pUserData, pOriginal);
        return nullptr;
    }

    void* originalPtr = static_cast<char*>(pOriginal) - sizeof(AllocationHeader);
    auto* header = static_cast<AllocationHeader*>(originalPtr);
    size_t oldSize = header->size;

    void* newPtr = allocation(pUserData, size, alignment, allocationScope);
    if (newPtr) {
        std::memcpy(newPtr, pOriginal, std::min(oldSize, size));
        free(pUserData, pOriginal);
    }

    return newPtr;
}

VKAPI_ATTR void VKAPI_CALL VulkanAllocator::free(
    void* pUserData,
    void* pMemory) noexcept {
    
    if (!pMemory) return;

    auto* self = static_cast<VulkanAllocator*>(pUserData);
    void* originalPtr = static_cast<char*>(pMemory) - sizeof(AllocationHeader);
    auto* header = static_cast<AllocationHeader*>(originalPtr);
    
    self->totalAllocated.fetch_sub(header->size, std::memory_order_relaxed);

#if defined(_MSC_VER) || defined(__MINGW32__)
    _aligned_free(originalPtr);
#else
    std::free(originalPtr);
#endif
}

} // namespace vnd
