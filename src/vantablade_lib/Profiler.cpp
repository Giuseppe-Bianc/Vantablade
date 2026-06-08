// TRACY_IMPLEMENTATION must be defined before the first inclusion of any Tracy
// header in this translation unit. Profiler.hpp includes Tracy.hpp transitively,
// so this define must appear before the Profiler.hpp include below.
//
// This makes the current translation unit own Tracy's compiled implementation.
// Exactly one TU in the program may define this.
//
// Alternative (preferred for larger projects): remove this define entirely and
// instead add ${TRACY_DIR}/public/TracyClient.cpp to your CMake target_sources.
// That file handles everything and removes the ordering constraint.
#ifdef VANTABLADE_PROFILING
#define TRACY_IMPLEMENTATION
#endif

#include "Vantablade/Profiler.hpp"

namespace Vantablade {

#ifdef VANTABLADE_PROFILING

    VulkanProfiler::~VulkanProfiler() { shutdown(); }

    void VulkanProfiler::init(const InitParams &params) {
        LINFO("Profiler::init BEGIN");
        // Instance-level: usa vkGetInstanceProcAddr
        auto getCalibrateableTimeDomains = reinterpret_cast<PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT>(
            vkGetInstanceProcAddr(params.instance, "vkGetPhysicalDeviceCalibrateableTimeDomainsEXT"));

        // Device-level: vkGetDeviceProcAddr è corretto qui
        auto getCalibratedTimestamps = reinterpret_cast<PFN_vkGetCalibratedTimestampsEXT>(
            vkGetDeviceProcAddr(params.device, "vkGetCalibratedTimestampsEXT"));

        if(getCalibrateableTimeDomains && getCalibratedTimestamps) {
            m_tracyCtx = TracyVkContextCalibrated(params.physicalDevice, params.device, params.queue, params.cmdBuffer,
                                                  getCalibrateableTimeDomains, getCalibratedTimestamps);
        } else {
            m_tracyCtx = TracyVkContext(params.physicalDevice, params.device, params.queue, params.cmdBuffer);
        }

        if(m_tracyCtx && params.contextName) {
            const auto len = static_cast<uint16_t>(std::strlen(params.contextName));
            TracyVkContextName(m_tracyCtx, params.contextName, len);
        }
        LINFO("Profiler::init END, ctx={}", (void *)m_tracyCtx);
    }

    void VulkanProfiler::shutdown() {
        if(!m_tracyCtx) { return; }
        // GPU must be idle before this call. TracyVkDestroy reads back any pending
        // timestamp queries and frees Tracy's internal Vulkan resources.
        TracyVkDestroy(m_tracyCtx);
        m_tracyCtx = nullptr;
    }

    void VulkanProfiler::collect(VkCommandBuffer cmd) {
        if(!m_tracyCtx) { return; }
        // Records commands into cmd that will retrieve timestamp results when the
        // GPU executes this command buffer. No CPU stall. Results are forwarded
        // to Tracy asynchronously after the GPU completes the frame.
        TracyVkCollect(m_tracyCtx, cmd);
    }

#endif  // VANTABLADE_PROFILING

}  // namespace Vantablade
