#ifdef VANTABLADE_PROFILING
#define TRACY_IMPLEMENTATION
#include <tracy/Tracy.hpp>
#include <tracy/TracyVulkan.hpp>
#endif

#include "Vantablade/Profiler.hpp"

namespace Vantablade {

#ifdef VANTABLADE_PROFILING

    VulkanProfiler::~VulkanProfiler() {
        if(m_queryPool) { vkDestroyQueryPool(m_device, m_queryPool, nullptr); }
    }

    void VulkanProfiler::init(VkDevice device, VkPhysicalDevice physicalDevice) {
        m_device = device;

        TracyVulkan::SetDevice(device);

        VkPhysicalDeviceProperties deviceProps = {};
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProps);
        m_timestampPeriod = deviceProps.limits.timestampPeriod;

        VkQueryPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        poolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
        poolInfo.queryCount = 1024;

        VK_CHECK(vkCreateQueryPool(m_device, &poolInfo, nullptr, &m_queryPool), "VulkanProfiler: Failed to create query pool")
    }

    void VulkanProfiler::mapQueue(uint32_t queueIndex, VkQueue queue) { TracyVulkan::SetQueue(queueIndex, queue); }

    void VulkanProfiler::beginGpuZone(VkCommandBuffer cmd, const char *name) {
        if(!m_queryPool) return;

        uint32_t queryIdx = m_currentQueryIndex % 1024;
        vkCmdWriteTimestamp(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, m_queryPool, queryIdx);

        m_activeZones.push_back({queryIdx, 0, name});
        m_currentQueryIndex++;
    }

    void VulkanProfiler::endGpuZone(VkCommandBuffer cmd) {
        if(!m_queryPool || m_activeZones.empty()) return;

        uint32_t queryIdx = m_currentQueryIndex % 1024;
        vkCmdWriteTimestamp(cmd, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, m_queryPool, queryIdx);

        m_activeZones.back().endQuery = queryIdx;
        m_currentQueryIndex++;
    }

    void VulkanProfiler::resolveTimestamps() {
        if(!m_queryPool || m_activeZones.empty()) return;

        for(const auto &zone : m_activeZones) {
            uint64_t start, end;
            vkGetQueryPoolResults(m_device, m_queryPool, zone.startQuery, 1, &start, sizeof(uint64_t),
                                  VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT);
            vkGetQueryPoolResults(m_device, m_queryPool, zone.endQuery, 1, &end, sizeof(uint64_t),
                                  VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT);

            double durationMs = (double)(end - start) * m_timestampPeriod * 1e-6;
            VZ_PLOT_FLOAT(zone.name, (float)durationMs);
        }
        m_activeZones.clear();
        // Reset query pool if we've used most of it, or just let it wrap around if we're careful.
        // For simplicity, we just wrap. If we exceed 1024 queries per frame, we need a bigger pool.
    }

#endif

}  // namespace Vantablade
