/*
 * Created by gbian on 01/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "vulkanCheck.hpp"
#include <vulkan/vk_enum_string_helper.h>

DISABLE_WARNINGS_PUSH(26429 26481)

// PERF: std::string_view — zero-copy, zero-allocation for string literals and existing std::string.
// The previous const std::string& forced a heap allocation for every literal call site
// (e.g., "--- Queue Labels ---", "================================================================================").
inline static void printMessageWhitSeverity(std::string_view msg, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity) {
    switch(messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        LTRACE(msg);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        LINFO(msg);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        LWARN(msg);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        LERROR(msg);
        break;
    default:
        LDEBUG(msg);
        break;
    }
}

inline static void logQueueLabel(const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                 VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity) {
    if(pCallbackData->queueLabelCount > 0) {
        printMessageWhitSeverity("--- Queue Labels ---", messageSeverity);
        for(uint32_t i = 0; i < pCallbackData->queueLabelCount; ++i) {
            const std::string_view labelName = pCallbackData->pQueueLabels[i].pLabelName ? pCallbackData->pQueueLabels[i].pLabelName
                                                                                         : "Unknown";
            const auto msg = FORMAT("  [{}] Label: {}", i, labelName);
            printMessageWhitSeverity(msg, messageSeverity);
        }
    }
}

/**
 * @brief Logs Command Buffer labels associated with the message.
 */
inline static void logCmdBuffers(const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                 VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity) {
    if(pCallbackData->cmdBufLabelCount > 0) {
        printMessageWhitSeverity("--- Command Buffer Labels ---", messageSeverity);
        for(uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; ++i) {
            const std::string_view labelName = pCallbackData->pCmdBufLabels[i].pLabelName ? pCallbackData->pCmdBufLabels[i].pLabelName
                                                                                          : "Unknown";
            const auto msg = FORMAT("  [{}] Label: {} {{ RGBA: {}, {}, {}, {} }}", i, labelName, pCallbackData->pCmdBufLabels[i].color[0],
                                    pCallbackData->pCmdBufLabels[i].color[1], pCallbackData->pCmdBufLabels[i].color[2],
                                    pCallbackData->pCmdBufLabels[i].color[3]);
            printMessageWhitSeverity(msg, messageSeverity);
        }
    }
}

/**
 * @brief Logs Vulkan objects related to the debug message.
 */
inline static void logObjects(const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                              VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity) {
    if(pCallbackData->objectCount > 0) {
        printMessageWhitSeverity("--- Related Objects ---", messageSeverity);
        for(uint32_t i = 0; i < pCallbackData->objectCount; ++i) {
            const std::string_view objectName = pCallbackData->pObjects[i].pObjectName ? pCallbackData->pObjects[i].pObjectName : "Unknown";
            const auto objectType = pCallbackData->pObjects[i].objectType;
            const auto objhandle = pCallbackData->pObjects[i].objectHandle;
            const auto objtypestring = string_VkObjectType(objectType);
            std::string msg;
            // NOLINTBEGIN(*-pro-type-reinterpret-cast, *-no-int-to-ptr)
            switch(objectType) {
            case VK_OBJECT_TYPE_INSTANCE:
            case VK_OBJECT_TYPE_PHYSICAL_DEVICE:
            case VK_OBJECT_TYPE_DEVICE:
            case VK_OBJECT_TYPE_COMMAND_BUFFER:
            case VK_OBJECT_TYPE_QUEUE:
                msg = FORMAT("  [{}] Type: {} | Handle: {} | Name: {}", i, objtypestring,
                             reinterpret_cast<void *>(static_cast<uintptr_t>(objhandle)), objectName);
                break;
            default:
                msg = FORMAT("  [{}] Type: {} | Handle: 0x{:X} | Name: {}", i, objtypestring, objhandle, objectName);
                break;
            }
            // NOLINTEND(*-pro-type-reinterpret-cast, *-no-int-to-ptr)
            printMessageWhitSeverity(msg, messageSeverity);
        }
    }
}

inline void logDebugValidationLayerInfo(const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity) {
    logQueueLabel(pCallbackData, messageSeverity);
    logCmdBuffers(pCallbackData, messageSeverity);
    logObjects(pCallbackData, messageSeverity);
}

DISABLE_WARNINGS_POP()

// NOLINTEND(*-include-cleaner)