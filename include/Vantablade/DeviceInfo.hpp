/*
 * Created by gbian on 06/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */

/*
 * Created by gbian on 15/03/2025.
 * Copyright (c) 2025 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "headers.hpp"
#include "vulkanToString.hpp"

#include <vulkan/vk_enum_string_helper.h>

DISABLE_WARNINGS_PUSH(26446 26482)

[[nodiscard]] static inline std::string_view getVendorName(uint32_t vendorID) {
    static const std::unordered_map<uint32_t, std::string_view> vendorMap = {
        {0x1002, "Advanced Micro Devices, Inc. (AMD)"},
        {0x1010, "ImgTec"},
        {0x10DE, "NVIDIA Corporation"},
        {0x13B5, "ARM"},
        {0x5143, "Qualcomm"},
        {0x8086, "Intel Corporation"},
        {0x1A03, "ASPEED Technology"},
        {0x1D17, "Samsung Electronics Co Ltd"},
        {0x1E0F, "Huawei Technologies Co., Ltd."},
        {0x1B36, "Red Hat, Inc."},
        {0x1AF4, "Virtio"},
        {0x1C58, "JMicron Technology Corp."},
        {0x1106, "VIA Technologies, Inc."},
        {0x103C, "Hewlett-Packard Company"},
        {0x1022, "Advanced Micro Devices, Inc. (AMD)"},
        {0x102B, "Matrox Electronic Systems Ltd."},
        {0x1043, "ASUSTeK Computer Inc."},
        {0x1179, "Toshiba Corporation"},
        {0x11AB, "Marvell Technology Group Ltd."},
        {0x1237, "Intel Corporation (i440FX)"},
        {0x15B7, "SanDisk"},
        {0x168C, "Qualcomm Atheros"},
        {0x1912, "Renesas Electronics Corporation"},
        {0x1B4B, "Marvell Technology Group Ltd."},
        {0x1C5C, "Fresco Logic"},
        {0x1D6A, "Google, Inc."},
        {0x8087, "Intel Corporation"},
        {0x1057, "Motorola"},
        {0x1077, "QLogic Corp."},
        {0x1095, "Silicon Image, Inc."},
        {0x10EC, "Realtek Semiconductor Corp."},
        {0x11C1, "Lattice Semiconductor Corporation"},
        {0x14E4, "Broadcom Inc."},
        {0x15AD, "VMware, Inc."},
        {0x15BC, "Hitachi, Ltd."},
        {0x18D1, "Google Inc."},
        {0x1AE0, "Xilinx, Inc."},
        {0x1D0F, "Amazon.com, Inc."},
        {0x1C7C, "Tehuti Networks Ltd."},
        {0x16C3, "Cavium Inc."},
        {0x105A, "Promise Technology"},
        {0x12D8, "Pericom Semiconductor"},
        {0x1B21, "ASMedia Technology Inc."},
        {0x126F, "Silicon Motion, Inc."},
        {0x1137, "Cisco Systems Inc."},
        {0x1DB1, "Lite-On Technology Corporation"},
        {0x10B5, "PLX Technology, Inc."},
        {0x10B7, "3Com Corporation"},
        {0x15E8, "Solarflare Communications"},
        {0x1A3B, "Facebook, Inc."},
        {0x1CC4, "Innogrit, Inc."},
        {0x1C20, "Mellanox Technologies"},
    };

    const auto it = vendorMap.find(vendorID);
    return it != vendorMap.end() ? it->second : std::string_view{"Unknown Vendor"};
}

static inline const char *getDeviceType(VkPhysicalDeviceType input_value) noexcept {
    switch(input_value) {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
        return "OTHER";
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        return "INTEGRATED_GPU";
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        return "DISCRETE_GPU";
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        return "VIRTUAL_GPU";
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        return "CPU";
    default:
        return "Unhandled VkPhysicalDeviceType";
    }
}

static inline void printDeviceFeatures(VkPhysicalDevice device) {
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);

    LINFO("=== Supported Features ===");
    LINFO("Geometry Shader:       {}", C_BOOL(features.geometryShader));
    LINFO("Tessellation Shader:   {}", C_BOOL(features.tessellationShader));
    LINFO("Multi Viewport:        {}", C_BOOL(features.multiViewport));
    LINFO("Sampler Anisotropy:    {}", C_BOOL(features.samplerAnisotropy));
    LINFO("Texture Compression:   {}", C_BOOL(features.textureCompressionBC));
    // Aggiungi altre features...
}

static inline std::string formatDeviceSize(VkDeviceSize size) {
    const FileSizeReport report{
        .info = {.bytes = size},
        .si_sys = kSI,
        .iec_sys = kIEC,
    };

    return FORMAT("Bytes: {:<10} SI({}) IEC({})", report.info.bytes, report.info.format(report.si_sys), report.info.format(report.iec_sys));
}

static inline void printMemoryInfo(VkPhysicalDevice device) {
    VkPhysicalDeviceMemoryProperties mem_props;
    vkGetPhysicalDeviceMemoryProperties(device, &mem_props);

    const auto mhc = C_ST(mem_props.memoryHeapCount);
    LINFO("=== Memory Heaps ({} total) ===", mhc);
    for(size_t i = 0; i < mhc; ++i) {
        const auto size = formatDeviceSize(mem_props.memoryHeaps[i].size);
        const auto propertyFlags = mem_props.memoryHeaps[i].flags;

        LINFO("Heap {:2}: {} {}", i, size, VkMemoryPropertyFlagsString(propertyFlags));
    }

    const auto mtc = C_ST(mem_props.memoryTypeCount);
    LINFO("=== Memory Types ({} total) ===", mtc);
    for(size_t i = 0; i < mtc; ++i) {
        const auto &[propertyFlags, heapIndex] = mem_props.memoryTypes[i];

        LINFO("Type {:2}: Heap {:2} | {}", i, heapIndex, VkMemoryPropertyFlagsString(propertyFlags));
    }
}

static inline void printQueueFamilies(VkPhysicalDevice device) {
    uint32_t count{};
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
    std::vector<VkQueueFamilyProperties> queues(count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queues.data());

    LINFO("=== Queue Families ({} total) ===", count);
    for(uint32_t i = 0; i < queues.size(); ++i) {
        const auto &q = queues[i];
        LINFO("Family {:2}: {:2} queues | {}", i, q.queueCount, VkQueueFlagsString(q.queueFlags));
    }
}

[[nodiscard]] static inline std::string uuid_to_string(std::span<const uint8_t, 16> uuid) {
    return FORMAT("{:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}"
                  "-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
                  uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7], uuid[8], uuid[9], uuid[10], uuid[11], uuid[12],
                  uuid[13], uuid[14], uuid[15]);
}

static inline void printPhysicalDeviceProperties(const VkPhysicalDeviceProperties &properties) {
    LINFO("Device Name: {}", properties.deviceName);
    const auto apiVersion = properties.apiVersion;
    const auto driverVersion = properties.driverVersion;
    LINFO("API Version: {}.{}.{}", VK_API_VERSION_MAJOR(apiVersion), VK_API_VERSION_MINOR(apiVersion), VK_API_VERSION_PATCH(apiVersion));
    LINFO("Driver Version: {}.{}.{}", VK_API_VERSION_MAJOR(driverVersion), VK_API_VERSION_MINOR(driverVersion),
          VK_API_VERSION_PATCH(driverVersion));
    LINFO("Vendor: {} (ID: {:#010x})", getVendorName(properties.vendorID), properties.vendorID);
    LINFO("Device ID: {0}({0:#010x})", properties.deviceID);
    LINFO("Device Type: {}", getDeviceType(properties.deviceType));
    LINFO("pipelineCacheUUID: {}", uuid_to_string(properties.pipelineCacheUUID));

    // Add more properties as needed
}

static inline void printDeviceInfo([[maybe_unused]] VkPhysicalDevice device, const VkPhysicalDeviceProperties &properties) {
    printPhysicalDeviceProperties(properties);
    // string_VkDriverId
    // printExtendedVulkanProperties(device);
#ifndef NDEBUG
    printDeviceFeatures(device);
    printMemoryInfo(device);
    printQueueFamilies(device);
#endif
    // printDeviceExtensions(device);
}
DISABLE_WARNINGS_POP()

// NOLINTEND(*-include-cleaner)