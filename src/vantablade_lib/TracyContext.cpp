/*
 * Created by gbian on 13/06/2026.
 * Copyright (c) 2026 All rights reserved.
 */

#include "Vantablade/TracyContext.hpp"

namespace Vantablade {

#ifdef TRACY_ENABLE

    bool TracyContext::initVulkan(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device) {
        // Caricamento dinamico delle funzioni Vulkan richieste per il path Host-Reset (Tracy v0.13.x)

        // vkResetQueryPool è richiesta per TracyVkCollectHost (Vulkan 1.2+ o VK_EXT_host_query_reset)
        auto vkResetQueryPool = reinterpret_cast<PFN_vkResetQueryPool>(vkGetDeviceProcAddr(device, "vkResetQueryPool"));

        // Funzioni per la calibrazione dei timestamp CPU-GPU
        auto vkGetPhysicalDeviceCalibrateableTimeDomainsEXT = reinterpret_cast<PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT>(
            vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceCalibrateableTimeDomainsEXT"));

        auto vkGetCalibratedTimestampsEXT = reinterpret_cast<PFN_vkGetCalibratedTimestampsEXT>(
            vkGetDeviceProcAddr(device, "vkGetCalibratedTimestampsEXT"));

        // Verifica che tutte le funzioni necessarie siano disponibili
        if(vkResetQueryPool && vkGetPhysicalDeviceCalibrateableTimeDomainsEXT && vkGetCalibratedTimestampsEXT) {
            // Path Moderno (Vulkan 1.2+ / VK_EXT_host_query_reset):
            // Non richiede code o buffer di comando per il reset del pool di query
            m_vkCtx = TracyVkContextHostCalibrated(physicalDevice, device, vkResetQueryPool, vkGetPhysicalDeviceCalibrateableTimeDomainsEXT,
                                                   vkGetCalibratedTimestampsEXT);

            if(!m_vkCtx) {
                LWARN("Fallimento inizializzazione TracyVkContextHostCalibrated.");
                return false;
            }

            LINFO("Tracy Vulkan Context inizializzato con successo (Host-Reset Path).");
            return true;
        }

        LWARN("Funzioni Host-Reset non supportate. Tracy GPU profiling disabilitato per questa sessione.");
        return false;
    }

    void TracyContext::collectVulkan() {
        if(m_vkCtx) {
            // TracyVkCollectHost è disponibile per il path host-reset.
            // Non richiede un VkCommandBuffer, a differenza del legacy TracyVkCollect.
            // Esegue la lettura dei timestamp GPU e il reset del pool di query via CPU.
            TracyVkCollectHost(m_vkCtx);
        }
    }

    void TracyContext::markFrame() { FrameMark; }

    TracyContext::~TracyContext() {
        if(m_vkCtx) {
            TracyVkDestroy(m_vkCtx);
            m_vkCtx = nullptr;
        }
    }

#else
    // =====================================================================
    // PATH NO-OP (Quando TRACY_ENABLE non è definito)
    // Il compilatore ottimizzerà via queste funzioni (inline/empty).
    // =====================================================================

    bool TracyContext::initVulkan(VkInstance, VkPhysicalDevice, VkDevice) { return true; }

    void TracyContext::collectVulkan() {}

    void TracyContext::markFrame() {}

    TracyContext::~TracyContext() = default;

#endif

}  // namespace Vantablade