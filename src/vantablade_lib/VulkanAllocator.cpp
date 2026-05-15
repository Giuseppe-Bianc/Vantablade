// clang-format off
// NOLINTBEGIN(*-include-cleaner,*-no-malloc, *-owning-memory, *-pro-type-reinterpret-cast, *-pro-bounds-pointer-arithmetic, *-uppercase-literal-suffix, *-macro-usage)
// clang-format on
#include "Vantablade/VulkanAllocator.hpp"

#if __has_cpp_attribute(assume) >= 202207L
#define VND_ASSUME(expr) [[assume(expr)]]
#elif defined(_MSC_VER)
#define VND_ASSUME(expr) __assume(expr)
#elif defined(__clang__)
#define VND_ASSUME(expr) __builtin_assume(expr)
#elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5))
#define VND_ASSUME(expr)                                                                                                                   \
    do {                                                                                                                                   \
        if(!(expr)) __builtin_unreachable();                                                                                               \
    } while(0)
#else
#define VND_ASSUME(expr) ((void)0)  // No-op fallback for unsupported compilers
#endif

namespace vnd {


    void VulkanAllocator::updatePeak(std::atomic<std::size_t> &peak, std::size_t candidate) noexcept {
        std::size_t old = peak.load(std::memory_order_relaxed);
        while(candidate > old && !peak.compare_exchange_weak(old, candidate, std::memory_order_relaxed, std::memory_order_relaxed)) {}
    }

    VulkanAllocator::ScopeSnapshot VulkanAllocator::getScopeSnapshot(VkSystemAllocationScope scope) const noexcept {
        const auto idx = scopeIndex(scope);
        const auto &stats = scopes.at(idx);

        return ScopeSnapshot{
            .liveBytes = stats.liveBytes.load(std::memory_order_relaxed),
            .peakBytes = stats.peakBytes.load(std::memory_order_relaxed),
            .allocCount = stats.allocCount.load(std::memory_order_relaxed),
            .freeCount = stats.freeCount.load(std::memory_order_relaxed),
            .reallocCount = stats.reallocCount.load(std::memory_order_relaxed),
            .failCount = stats.failCount.load(std::memory_order_relaxed),
        };
    }

    constexpr std::string_view VulkanAllocator::scopeName(VkSystemAllocationScope scope) noexcept {
        // C++23: std::to_underlying for enum class conversion
        switch(scope) {
        case VK_SYSTEM_ALLOCATION_SCOPE_COMMAND:
            return "COMMAND";
        case VK_SYSTEM_ALLOCATION_SCOPE_OBJECT:
            return "OBJECT";
        case VK_SYSTEM_ALLOCATION_SCOPE_CACHE:
            return "CACHE";
        case VK_SYSTEM_ALLOCATION_SCOPE_DEVICE:
            return "DEVICE";
        case VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE:
            return "INSTANCE";
        default:
            return "UNKNOWN";  // Forward-compatibility for future Vulkan extensions
        }
    }

    VKAPI_ATTR void *VKAPI_CALL VulkanAllocator::vklAllocation(void *pUserData, std::size_t size, std::size_t alignment,
                                                               VkSystemAllocationScope allocationScope) noexcept {
        // Vulkan §11.8: size == 0 must return nullptr
        if(size == 0u) [[unlikely]] { return nullptr; }

        // Vulkan §11.8 guarantee: alignment is power of two and > 0
        // [[assume]] communicates invariant to optimizer without runtime cost
        VND_ASSUME(alignment > 0u && (alignment & (alignment - 1u)) == 0u);

        auto *const self = static_cast<VulkanAllocator *>(pUserData);
        VND_ASSUME(self != nullptr);  // Vulkan guarantees pUserData is the allocator instance

        // Handle unknown scopes: log warning, fallback to index 0 (forward-compatibility)
        if(!isKnownScope(allocationScope)) [[unlikely]] {
            LERROR("Vulkan CPU allocation with unknown scope (scope={})", std::to_underlying(allocationScope));
        }

        const std::size_t effectiveAlign = computeEffectiveAlignment(alignment);

        // Calculate total malloc size: header + padding + payload
        // Overflow checks per C++ Core Guidelines ES.103
        std::size_t overhead{};
        if(!checkedAdd(sizeof(AllocationHeader), effectiveAlign - 1u, overhead)) [[unlikely]] {
            self->scopes.at(scopeIndex(allocationScope)).failCount.fetch_add(1, std::memory_order_relaxed);
            LERROR("Vulkan CPU allocation overhead overflow (size={}, alignment={}, scope={})", size, alignment,
                   std::to_underlying(allocationScope));
            return nullptr;
        }

        std::size_t totalSize{};
        if(!checkedAdd(overhead, size, totalSize)) [[unlikely]] {
            self->scopes.at(scopeIndex(allocationScope)).failCount.fetch_add(1, std::memory_order_relaxed);
            LERROR("Vulkan CPU allocation totalSize overflow (size={}, alignment={}, scope={})", size, alignment,
                   std::to_underlying(allocationScope));
            return nullptr;
        }

        // Allocate raw memory block
        void *const base = std::malloc(totalSize);
        if(base == nullptr) [[unlikely]] {
            self->scopes.at(scopeIndex(allocationScope)).failCount.fetch_add(1, std::memory_order_relaxed);
            LERROR("Vulkan CPU allocation failed (size={}, alignment={}, scope={})", size, alignment, std::to_underlying(allocationScope));
            return nullptr;
        }

        // Compute aligned user pointer using std::align (C++11, constexpr in C++23)
        // Space calculation: totalSize - header size = padding + payload space
        void *userPtr = static_cast<char *>(base) + sizeof(AllocationHeader);
        std::size_t space = totalSize - sizeof(AllocationHeader);

        // std::align advances userPtr to first effectiveAlign-aligned address
        // Cannot fail: totalSize guarantees sufficient room (invariant documented above)
        [[maybe_unused]] void *const alignResult = std::align(effectiveAlign, size, userPtr, space);
        assert(alignResult != nullptr);  // Debug-mode invariant check

        // Place header immediately before aligned user pointer
        // Pointer arithmetic on char* is well-defined within same malloc block (C++ [expr.add])
        auto *const header = reinterpret_cast<AllocationHeader *>(static_cast<char *>(userPtr) - sizeof(AllocationHeader));

        // AllocationHeader is trivially constructible; no placement-new needed (C++23 [intro.object])
        header->size = size;
        header->base = base;
        header->scope = allocationScope;

#if VND_ALLOCATOR_DEBUG
        // Poison payload in debug mode to detect use-after-free/uninitialized reads
        poisonMemory(userPtr, size);
#endif

        // Update statistics (lock-free, memory_order_relaxed)
        self->totalAllocated.fetch_add(size, std::memory_order_relaxed);

        auto &stats = self->scopes.at(scopeIndex(allocationScope));
        stats.allocCount.fetch_add(1, std::memory_order_relaxed);

        const std::size_t newLive = stats.liveBytes.fetch_add(size, std::memory_order_relaxed) + size;
        updatePeak(stats.peakBytes, newLive);

#if VND_ALLOCATOR_TELEMETRY
        // Optional telemetry callback for external monitoring
        if(auto cb = self->telemetryCallback.load(std::memory_order_relaxed); cb != nullptr) {
            cb(self->telemetryUserData.load(std::memory_order_relaxed), "alloc", size, allocationScope);
        }
#endif

        return userPtr;
    }

    VKAPI_ATTR void *VKAPI_CALL VulkanAllocator::vklReallocation(void *pUserData, void *pOriginal, std::size_t size, std::size_t alignment,
                                                                 VkSystemAllocationScope allocationScope) noexcept {
        // Vulkan §11.8: pOriginal == nullptr → behave as allocation
        if(pOriginal == nullptr) [[unlikely]] { return vklAllocation(pUserData, size, alignment, allocationScope); }

        // Vulkan §11.8: size == 0 → behave as free, return nullptr
        if(size == 0u) [[unlikely]] {
            vklFree(pUserData, pOriginal);
            return nullptr;
        }

        auto *const self = static_cast<VulkanAllocator *>(pUserData);
        self->scopes.at(scopeIndex(allocationScope)).reallocCount.fetch_add(1, std::memory_order_relaxed);

        // Recover old size from header (pointer arithmetic within same malloc block)
        const auto *const oldHeader = reinterpret_cast<const AllocationHeader *>(static_cast<char *>(pOriginal) - sizeof(AllocationHeader));
        const std::size_t oldSize = oldHeader->size;

#if VND_ALLOCATOR_DEBUG
        // Verify old payload hasn't been corrupted (detects buffer overflows)
        if(!verifyMemory(pOriginal, oldSize)) [[unlikely]] { LERROR("Memory corruption detected in reallocation (expected pattern 0xDE)"); }
#endif

        // Allocate new block (may fail; old block remains valid if so)
        void *const newPtr = vklAllocation(pUserData, size, alignment, allocationScope);

        if(newPtr != nullptr) [[likely]] {
            // Copy min(oldSize, size) bytes: Vulkan §11.8 specifies this behavior
            // std::memcpy is safe: old and new blocks are disjoint (independent malloc calls)
            std::memcpy(newPtr, pOriginal, std::min(oldSize, size));

            // Free old block (updates statistics, calls std::free)
            vklFree(pUserData, pOriginal);
        }
        // If allocation failed: pOriginal remains valid, return nullptr per Vulkan contract

        return newPtr;
    }

    // NOLINTNEXTLINE(*-easily-swappable-parameters)
    VKAPI_ATTR void VKAPI_CALL VulkanAllocator::vklFree(void *pUserData, void *pMemory) noexcept {
        // Vulkan §11.8: pMemory == nullptr must be no-op
        if(pMemory == nullptr) [[unlikely]] { return; }

        auto *const self = static_cast<VulkanAllocator *>(pUserData);

        // Recover header from bytes immediately before user pointer
        const auto *const header = reinterpret_cast<const AllocationHeader *>(static_cast<char *>(pMemory) - sizeof(AllocationHeader));

        const std::size_t sz = header->size;
        const VkSystemAllocationScope scope = header->scope;

#if VND_ALLOCATOR_DEBUG
        // Poison memory before freeing to catch use-after-free
        poisonMemory(pMemory, sz);
#endif

        // Update statistics (lock-free)
        self->totalAllocated.fetch_sub(sz, std::memory_order_relaxed);

        auto &stats = self->scopes.at(scopeIndex(scope));
        stats.freeCount.fetch_add(1, std::memory_order_relaxed);
        stats.liveBytes.fetch_sub(sz, std::memory_order_relaxed);

#if VND_ALLOCATOR_TELEMETRY
        // Optional telemetry for free events
        if(auto cb = self->telemetryCallback.load(std::memory_order_relaxed); cb != nullptr) {
            cb(self->telemetryUserData.load(std::memory_order_relaxed), "free", sz, scope);
        }
#endif

        // Free the original malloc block (header.base points to start of block)
        std::free(header->base);
    }

    void VulkanAllocator::dumpOneScope(VkSystemAllocationScope scope) const {
        const auto snapshot = getScopeSnapshot(scope);

        LINFO("[{}] live={} peak={} alloc={} free={} realloc={} fail={}", scopeName(scope), snapshot.liveBytes, snapshot.peakBytes,
              snapshot.allocCount, snapshot.freeCount, snapshot.reallocCount, snapshot.failCount);
    }

    void VulkanAllocator::dumpReport() const {
        const std::size_t totalLive = getTotalAllocated();

        LINFO("VulkanAllocator CPU Memory Report");

        // Dump per-scope statistics
        dumpOneScope(VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
        dumpOneScope(VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
        dumpOneScope(VK_SYSTEM_ALLOCATION_SCOPE_CACHE);
        dumpOneScope(VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
        dumpOneScope(VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);

        const std::size_t snapshot0 = getScopeSnapshot(VK_SYSTEM_ALLOCATION_SCOPE_COMMAND).liveBytes;
        const std::size_t snapshot1 = getScopeSnapshot(VK_SYSTEM_ALLOCATION_SCOPE_OBJECT).liveBytes;
        const std::size_t snapshot2 = getScopeSnapshot(VK_SYSTEM_ALLOCATION_SCOPE_CACHE).liveBytes;
        const std::size_t snapshot3 = getScopeSnapshot(VK_SYSTEM_ALLOCATION_SCOPE_DEVICE).liveBytes;
        const std::size_t snapshot4 = getScopeSnapshot(VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE).liveBytes;

        const std::size_t scopesSum = snapshot0 + snapshot1 + snapshot2 + snapshot3 + snapshot4;
        const std::size_t delta = (scopesSum >= totalLive) ? (scopesSum - totalLive) : (totalLive - scopesSum);

        const FileSizeReport totalLiveReport{
            .info = {.bytes = totalLive},
            .si_sys = kSI,
            .iec_sys = kIEC,
        };
        const FileSizeReport scopesSumReport{
            .info = {.bytes = scopesSum},
            .si_sys = kSI,
            .iec_sys = kIEC,
        };
        const FileSizeReport deltaReport{
            .info = {.bytes = delta},
            .si_sys = kSI,
            .iec_sys = kIEC,
        };
        LINFO("total_live   Bytes: {:<10} SI({}) IEC({})", totalLiveReport.info.bytes, totalLiveReport.info.format(totalLiveReport.si_sys),
              totalLiveReport.info.format(totalLiveReport.iec_sys));
        LINFO("scopes_sum   Bytes: {:<10} SI({}) IEC({})", scopesSumReport.info.bytes, scopesSumReport.info.format(scopesSumReport.si_sys),
              scopesSumReport.info.format(scopesSumReport.iec_sys));
        LINFO("delta        Bytes: {:<10} SI({}) IEC({})", deltaReport.info.bytes, deltaReport.info.format(deltaReport.si_sys),
              deltaReport.info.format(deltaReport.iec_sys));
    }

}  // namespace vnd

// clang-format off
// NOLINTEND(*-include-cleaner,*-no-malloc, *-owning-memory, *-pro-type-reinterpret-cast, *-pro-bounds-pointer-arithmetic, *-uppercase-literal-suffix, *-macro-usage)
// clang-format on
