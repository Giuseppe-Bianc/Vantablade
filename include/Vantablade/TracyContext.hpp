/*
 * Created by gbian on 13/06/2026.
 * Copyright (c) 2026 All rights reserved.
 */
#pragma once

#include "headers.hpp"

#ifdef TRACY_ENABLE
// ⚠️ CRITICO: L'ordine di inclusione è fondamentale per Tracy >= v0.8.0
// Vulkan headers DEVONO essere inclusi PRIMA di Tracy headers
#include <tracy/Tracy.hpp>
#include <tracy/TracyVulkan.hpp>
#endif

namespace Vantablade {

    /**
     * @brief Wrapper RAII per il contesto di profiling Tracy Profiler.
     *
     * Questa classe gestisce il ciclo di vita del contesto Vulkan di Tracy,
     * garantendo che le risorse vengano rilasciate nell'ordine corretto.
     *
     * Ordine di distruzione C++: Questo oggetto deve essere distrutto PRIMA
     * del VkDevice, poiché TracyVkDestroy chiama vkDestroyQueryPool che
     * richiede un device valido.
     */
    class TracyContext {
    public:
        TracyContext() = default;
        ~TracyContext();

        // Impedisce copia e assegnamento per evitare doppi rilasci
        TracyContext(const TracyContext &) = delete;
        TracyContext &operator=(const TracyContext &) = delete;
        TracyContext(TracyContext &&) = delete;
        TracyContext &operator=(TracyContext &&) = delete;

        /**
         * @brief Inizializza il contesto Vulkan di Tracy.
         *
         * Richiede Vulkan 1.2+ con hostQueryReset o l'estensione VK_EXT_host_query_reset.
         * Utilizza il path moderno TracyVkContextHostCalibrated per evitare l'uso di
         * command buffer per la raccolta dei dati.
         *
         * @param instance VkInstance valida
         * @param physicalDevice VkPhysicalDevice valida
         * @param device VkDevice valida
         * @return true se l'inizializzazione è riuscita, false altrimenti
         */
        [[nodiscard]] bool initVulkan(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);

        /**
         * @brief Raccoglie i dati GPU e resetta il pool di query.
         *
         * Deve essere chiamata dopo che il lavoro GPU è stato sincronizzato
         * (es. dopo vkWaitForFences). Utilizza TracyVkCollectHost per il
         * reset lato CPU, eliminando la necessità di command buffer dedicati.
         */
        void collectVulkan();

        /**
         * @brief Marca il completamento di un frame nella timeline di Tracy.
         */
        void markFrame();

#ifdef TRACY_ENABLE
        /**
         * @brief Restituisce il contesto Vulkan di Tracy per l'uso con le macro TracyVkZone.
         *
         * @return tracy::VkCtx* Il contesto Vulkan, o nullptr se non inizializzato
         */
        [[nodiscard]] tracy::VkCtx *getVkCtx() const noexcept { return m_vkCtx; }
#endif

    private:
#ifdef TRACY_ENABLE
        tracy::VkCtx *m_vkCtx = nullptr;
#endif
    };

}  // namespace Vantablade