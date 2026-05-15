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
#elif defined(__GNUC__)
#define VND_ASSUME(expr)                                                                                                                   \
    do {                                                                                                                                   \
        if(!(expr)) __builtin_unreachable();                                                                                               \
    } while(0)
#else
#define VND_ASSUME(expr) ((void)0)
#endif

namespace vnd {

    namespace {

        struct alignas(alignof(void *)) AllocationHeader final {
            std::size_t size{};
            std::size_t alignment{};
            void *base{};
            VkSystemAllocationScope scope{};
        };

        [[nodiscard]] constexpr std::size_t computeEffectiveAlignment(std::size_t alignment) noexcept {
            return std::max(alignment, alignof(AllocationHeader));
        }

        [[nodiscard]] constexpr bool checked_add(std::size_t lhs, std::size_t rhs, std::size_t &result) noexcept {
            if(rhs > std::numeric_limits<std::size_t>::max() - lhs) { return false; }
            result = lhs + rhs;
            return true;
        }

        [[nodiscard]] constexpr bool isKnownScope(VkSystemAllocationScope scope) noexcept {
            const auto scopeValue = static_cast<std::size_t>(std::to_underlying(scope));
            return scopeValue < VulkanAllocator::scopeCount;
        }

        [[nodiscard]] constexpr bool isPow2(std::size_t x) noexcept { return x != 0u && (x & (x - 1u)) == 0u; }

    }  // namespace

    void VulkanAllocator::updatePeak(std::atomic<std::size_t> &peak, std::size_t candidate) noexcept {
        std::size_t old = peak.load(std::memory_order_relaxed);
        while(candidate > old && !peak.compare_exchange_weak(old, candidate, std::memory_order_relaxed, std::memory_order_relaxed)) {}
    }

    std::size_t VulkanAllocator::getTotalLiveBytes() const noexcept { return totalLiveBytes_.load(std::memory_order_relaxed); }

    std::size_t VulkanAllocator::getTotalCumulativeAllocatedBytes() const noexcept {
        return totalCumulativeAllocatedBytes_.load(std::memory_order_relaxed);
    }

    VulkanAllocator::ScopeSnapshot VulkanAllocator::getScopeSnapshot(VkSystemAllocationScope scope) const noexcept {
        const auto &s = scopes_.at(scopeIndex(scope));
        return ScopeSnapshot{
            .liveBytes = s.liveBytes.load(std::memory_order_relaxed),
            .peakBytes = s.peakBytes.load(std::memory_order_relaxed),
            .allocCount = s.allocCount.load(std::memory_order_relaxed),
            .freeCount = s.freeCount.load(std::memory_order_relaxed),
            .reallocCount = s.reallocCount.load(std::memory_order_relaxed),
            .failCount = s.failCount.load(std::memory_order_relaxed),
            .internalAllocCount = s.internalAllocCount.load(std::memory_order_relaxed),
            .internalFreeCount = s.internalFreeCount.load(std::memory_order_relaxed),
            .internalLiveBytes = s.internalLiveBytes.load(std::memory_order_relaxed),
            .internalPeakBytes = s.internalPeakBytes.load(std::memory_order_relaxed),
        };
    }

    VKAPI_ATTR void *VKAPI_CALL VulkanAllocator::vklAllocation(void *pUserData, std::size_t size, std::size_t alignment,
                                                               VkSystemAllocationScope allocationScope) noexcept {
        if(size == 0u) { return nullptr; }

        // alignment deve essere power-of-two. :contentReference[oaicite:4]{index=4}
        VND_ASSUME(isPow2(alignment));

        auto *const self = static_cast<VulkanAllocator *>(pUserData);

        if(!isKnownScope(allocationScope)) {
            LERROR("Vulkan CPU allocation with unknown scope (scope={})", std::to_underlying(allocationScope));
        }

        const std::size_t effectiveAlign = computeEffectiveAlignment(alignment);

        std::size_t overhead{};
        if(!checked_add(sizeof(AllocationHeader), effectiveAlign - 1u, overhead)) {
            self->scopes_.at(scopeIndex(allocationScope)).failCount.fetch_add(1, std::memory_order_relaxed);
            LERROR("Vulkan CPU allocation overhead overflow (size={}, alignment={}, scope={})", size, alignment,
                   std::to_underlying(allocationScope));
            return nullptr;
        }

        std::size_t totalSize{};
        if(!checked_add(overhead, size, totalSize)) {
            self->scopes_.at(scopeIndex(allocationScope)).failCount.fetch_add(1, std::memory_order_relaxed);
            LERROR("Vulkan CPU allocation totalSize overflow (size={}, alignment={}, scope={})", size, alignment,
                   std::to_underlying(allocationScope));
            return nullptr;
        }

        void *const base = std::malloc(totalSize);
        if(base == nullptr) {
            self->scopes_.at(scopeIndex(allocationScope)).failCount.fetch_add(1, std::memory_order_relaxed);
            LERROR("Vulkan CPU allocation failed (size={}, alignment={}, scope={})", size, alignment, std::to_underlying(allocationScope));
            return nullptr;
        }

        void *userPtr = static_cast<char *>(base) + sizeof(AllocationHeader);
        std::size_t space = totalSize - sizeof(AllocationHeader);

        [[maybe_unused]] void *const alignResult = std::align(effectiveAlign, size, userPtr, space);
        assert(alignResult != nullptr);

        auto *const header = reinterpret_cast<AllocationHeader *>(static_cast<char *>(userPtr) - sizeof(AllocationHeader));

        header->size = size;
        header->alignment = alignment;
        header->base = base;
        header->scope = allocationScope;

        self->totalLiveBytes_.fetch_add(size, std::memory_order_relaxed);
        self->totalCumulativeAllocatedBytes_.fetch_add(size, std::memory_order_relaxed);

        auto &stats = self->scopes_.at(scopeIndex(allocationScope));
        stats.allocCount.fetch_add(1, std::memory_order_relaxed);

        const std::size_t live = stats.liveBytes.fetch_add(size, std::memory_order_relaxed) + size;
        updatePeak(stats.peakBytes, live);

        return userPtr;
    }

    VKAPI_ATTR void *VKAPI_CALL VulkanAllocator::vklReallocation(void *pUserData, void *pOriginal, std::size_t size, std::size_t alignment,
                                                                 VkSystemAllocationScope allocationScope) noexcept {
        // Regole Vulkan per realloc. :contentReference[oaicite:5]{index=5}
        if(pOriginal == nullptr) { return vklAllocation(pUserData, size, alignment, allocationScope); }
        if(size == 0u) {
            vklFree(pUserData, pOriginal);
            return nullptr;
        }

        auto *const self = static_cast<VulkanAllocator *>(pUserData);

        auto *const oldHeader = reinterpret_cast<const AllocationHeader *>(static_cast<char *>(pOriginal) - sizeof(AllocationHeader));

        // Vulkan richiede che alignment sia uguale a quello originale quando pOriginal != NULL. :contentReference[oaicite:6]{index=6}
        // In release non blocco, in debug segnalo. Se vuoi essere aggressivo, puoi abortire.
        if(oldHeader->alignment != alignment) {
            LERROR("Vulkan CPU realloc with mismatched alignment (old={}, new={}, scope={})", oldHeader->alignment, alignment,
                   std::to_underlying(allocationScope));
        }

        self->scopes_.at(scopeIndex(allocationScope)).reallocCount.fetch_add(1, std::memory_order_relaxed);

        const std::size_t oldSize = oldHeader->size;

        void *const newPtr = vklAllocation(pUserData, size, alignment, allocationScope);
        if(newPtr != nullptr) {
            std::memcpy(newPtr, pOriginal, std::ranges::min(oldSize, size));
            vklFree(pUserData, pOriginal);
        }
        return newPtr;
    }

    VKAPI_ATTR void VKAPI_CALL VulkanAllocator::vklFree(void *pUserData, void *pMemory) noexcept {
        if(pMemory == nullptr) { return; }

        auto *const self = static_cast<VulkanAllocator *>(pUserData);

        const auto *const header = reinterpret_cast<const AllocationHeader *>(static_cast<char *>(pMemory) - sizeof(AllocationHeader));

        const std::size_t sz = header->size;
        const VkSystemAllocationScope scope = header->scope;

        self->totalLiveBytes_.fetch_sub(sz, std::memory_order_relaxed);

        auto &stats = self->scopes_.at(scopeIndex(scope));
        stats.freeCount.fetch_add(1, std::memory_order_relaxed);
        stats.liveBytes.fetch_sub(sz, std::memory_order_relaxed);

        std::free(header->base);
    }

    VKAPI_ATTR void VKAPI_CALL VulkanAllocator::vklInternalAllocation(void *pUserData, std::size_t size,
                                                                      VkInternalAllocationType allocationType,
                                                                      VkSystemAllocationScope allocationScope) noexcept {
        (void)allocationType;  // Al momento non distinguiamo, ma la spec lo espone. :contentReference[oaicite:7]{index=7}
        auto *const self = static_cast<VulkanAllocator *>(pUserData);

        auto &stats = self->scopes_.at(scopeIndex(allocationScope));
        stats.internalAllocCount.fetch_add(1, std::memory_order_relaxed);

        const std::size_t live = stats.internalLiveBytes.fetch_add(size, std::memory_order_relaxed) + size;
        updatePeak(stats.internalPeakBytes, live);
    }

    VKAPI_ATTR void VKAPI_CALL VulkanAllocator::vklInternalFree(void *pUserData, std::size_t size, VkInternalAllocationType allocationType,
                                                                VkSystemAllocationScope allocationScope) noexcept {
        (void)allocationType;
        auto *const self = static_cast<VulkanAllocator *>(pUserData);

        auto &stats = self->scopes_.at(scopeIndex(allocationScope));
        stats.internalFreeCount.fetch_add(1, std::memory_order_relaxed);
        stats.internalLiveBytes.fetch_sub(size, std::memory_order_relaxed);
    }

    [[nodiscard]] constexpr std::string_view VulkanAllocator::scopeName(VkSystemAllocationScope scope) noexcept {
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
            return "UNKNOWN";
        }
    }

    void VulkanAllocator::dumpOneScope(VkSystemAllocationScope scope) const {
        const auto snap = getScopeSnapshot(scope);

        LINFO("[{}] live={} peak={} alloc={} free={} realloc={} fail={} int_live={} int_peak={} int_alloc={} int_free={}", scopeName(scope),
              snap.liveBytes, snap.peakBytes, snap.allocCount, snap.freeCount, snap.reallocCount, snap.failCount, snap.internalLiveBytes,
              snap.internalPeakBytes, snap.internalAllocCount, snap.internalFreeCount);
    }

    void VulkanAllocator::dumpReport() const {
        const std::size_t totalLive = getTotalLiveBytes();
        const std::size_t totalCum = getTotalCumulativeAllocatedBytes();

        LINFO("VulkanAllocator CPU Memory Report");
        LINFO("total_live_bytes={}", totalLive);
        LINFO("total_cumulative_allocated_bytes={}", totalCum);

        dumpOneScope(VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
        dumpOneScope(VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
        dumpOneScope(VK_SYSTEM_ALLOCATION_SCOPE_CACHE);
        dumpOneScope(VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
        dumpOneScope(VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);

        const std::size_t s0 = getScopeSnapshot(VK_SYSTEM_ALLOCATION_SCOPE_COMMAND).liveBytes;
        const std::size_t s1 = getScopeSnapshot(VK_SYSTEM_ALLOCATION_SCOPE_OBJECT).liveBytes;
        const std::size_t s2 = getScopeSnapshot(VK_SYSTEM_ALLOCATION_SCOPE_CACHE).liveBytes;
        const std::size_t s3 = getScopeSnapshot(VK_SYSTEM_ALLOCATION_SCOPE_DEVICE).liveBytes;
        const std::size_t s4 = getScopeSnapshot(VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE).liveBytes;

        const std::size_t scopesSum = s0 + s1 + s2 + s3 + s4;
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
// NOLINTEND(*-include-cleaner,*-no-malloc,*-owning-memory,*-pro-type-reinterpret-cast,*-pro-bounds-pointer-arithmetic,*-uppercase-literal-suffix,*-macro-usage)
// clang-format on