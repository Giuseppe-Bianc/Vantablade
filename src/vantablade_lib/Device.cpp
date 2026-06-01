/*
 * Created by gbian on 06/05/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// clang-format off
// NOLINTBEGIN(*-include-cleaner, *-signed-bitwise, *-easily-swappable-parameters, *-use-anonymous-namespace, *-diagnostic-old-style-cast, *-pro-type-cstyle-cast, *-pro-type-member-init,*-member-init, *-pro-bounds-constant-array-index, *-qualified-auto, *-uppercase-literal-suffix, *-identifier-length, *-magic-numbers, *-diagnostic-missing-designated-field-initializers)
// clang-format on
#include "Vantablade/Device.hpp"
#include "Vantablade/VkObjectTypeResolve.hpp"
#include "Vantablade/VulkanLogInfoCallback.hpp"

DISABLE_WARNINGS_PUSH(4100 4127 4189 4201 4324 4505 4820 26812)
#include <vk_mem_alloc.h>
DISABLE_WARNINGS_POP()
#include <cstdlib>

template <typename Fn> Fn Device::loadInstanceProc(VkInstance inst, const char *name) noexcept {
    // NOLINTNEXTLINE(*-pro-type-reinterpret-cast)
    return reinterpret_cast<Fn>(vkGetInstanceProcAddr(inst, name));
}

void Device::loadDebugUtilsFunctions() noexcept {
    if constexpr(!enableValidationLayers) { return; }
    const vnd::AutoTimer time{"load Debug Utils Functions"};

    debugFuncs.setObjectName = loadInstanceProc<PFN_vkSetDebugUtilsObjectNameEXT>(instance, "vkSetDebugUtilsObjectNameEXT");
    debugFuncs.cmdBeginLabel = loadInstanceProc<PFN_vkCmdBeginDebugUtilsLabelEXT>(instance, "vkCmdBeginDebugUtilsLabelEXT");
    debugFuncs.cmdEndLabel = loadInstanceProc<PFN_vkCmdEndDebugUtilsLabelEXT>(instance, "vkCmdEndDebugUtilsLabelEXT");
    debugFuncs.cmdInsertLabel = loadInstanceProc<PFN_vkCmdInsertDebugUtilsLabelEXT>(instance, "vkCmdInsertDebugUtilsLabelEXT");
    debugFuncs.queueBeginLabel = loadInstanceProc<PFN_vkQueueBeginDebugUtilsLabelEXT>(instance, "vkQueueBeginDebugUtilsLabelEXT");
    debugFuncs.queueEndLabel = loadInstanceProc<PFN_vkQueueEndDebugUtilsLabelEXT>(instance, "vkQueueEndDebugUtilsLabelEXT");
    debugFuncs.queueInsertLabel = loadInstanceProc<PFN_vkQueueInsertDebugUtilsLabelEXT>(instance, "vkQueueInsertDebugUtilsLabelEXT");

    LINFO("setObjectName   : {}", debugFuncs.setObjectName != nullptr);
    LINFO("cmdBeginLabel   : {}", debugFuncs.cmdBeginLabel != nullptr);
    LINFO("cmdEndLabel     : {}", debugFuncs.cmdEndLabel != nullptr);
    LINFO("cmdInsertLabel  : {}", debugFuncs.cmdInsertLabel != nullptr);
    LINFO("queueBeginLabel : {}", debugFuncs.queueBeginLabel != nullptr);
    LINFO("queueEndLabel   : {}", debugFuncs.queueEndLabel != nullptr);
    LINFO("queueInsertLabel: {}", debugFuncs.queueInsertLabel != nullptr);
}

// ---------------------------------------------------------------------------
// Helper interno: costruisce VkDebugUtilsLabelEXT senza heap allocation.
// Usato da tutti i wrapper che richiedono un colore.
// color è std::span<const float, 4>: extent statico garantito a compile time,
// zero-cost, accetta std::array, C-array e DebugLabelColor senza conversione.
// ---------------------------------------------------------------------------
[[nodiscard]] static VkDebugUtilsLabelEXT makeLabel(const char *name, std::span<const float, 4> color) noexcept {
    return VkDebugUtilsLabelEXT{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
        .pNext = nullptr,
        .pLabelName = name,
        .color = {color[0], color[1], color[2], color[3]},
    };
}

void Device::pcmdBeginLabel(VkCommandBuffer cb, const char *name, std::span<const float, 4> color) const noexcept {
    if constexpr(!enableValidationLayers) { return; }
    if(debugFuncs.cmdBeginLabel == nullptr) { return; }
    const auto label = makeLabel(name, color);
    debugFuncs.cmdBeginLabel(cb, &label);
}

void Device::pcmdEndLabel(VkCommandBuffer cb) const noexcept {
    if constexpr(!enableValidationLayers) { return; }
    if(debugFuncs.cmdEndLabel == nullptr) { return; }
    debugFuncs.cmdEndLabel(cb);
}

void Device::pcmdInsertLabel(VkCommandBuffer cb, const char *name, std::span<const float, 4> color) const noexcept {
    if constexpr(!enableValidationLayers) { return; }
    if(debugFuncs.cmdInsertLabel == nullptr) { return; }
    const auto label = makeLabel(name, color);
    debugFuncs.cmdInsertLabel(cb, &label);
}

void Device::pqueueBeginLabel(VkQueue queue, const char *name, std::span<const float, 4> color) const noexcept {
    if constexpr(!enableValidationLayers) { return; }
    if(debugFuncs.queueBeginLabel == nullptr) { return; }
    const auto label = makeLabel(name, color);
    debugFuncs.queueBeginLabel(queue, &label);
}

void Device::pqueueEndLabel(VkQueue queue) const noexcept {
    if constexpr(!enableValidationLayers) { return; }
    if(debugFuncs.queueEndLabel == nullptr) { return; }
    debugFuncs.queueEndLabel(queue);
}

void Device::pqueueInsertLabel(VkQueue queue, const char *name, std::span<const float, 4> color) const noexcept {
    if constexpr(!enableValidationLayers) { return; }
    if(debugFuncs.queueInsertLabel == nullptr) { return; }
    const auto label = makeLabel(name, color);
    debugFuncs.queueInsertLabel(queue, &label);
}

// local callback functions
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                    [[maybe_unused]] void *pUserData) {
    const char *id = pCallbackData->pMessageIdName;
    if(id != nullptr && std::string_view{id} == "Loader Message") { return VK_FALSE; }
    const std::string type = VkDebugUtilsMessageTypeFlagsEXTString(messageType);

    // Structured Header
    printMessageWhitSeverity("================================================================================", messageSeverity);
    const auto msg = FORMAT("{}Message ID: {}({}): {}", type, pCallbackData->pMessageIdName ? pCallbackData->pMessageIdName : "Unknown",
                            pCallbackData->messageIdNumber, pCallbackData->pMessage);
    // Primary Message
    printMessageWhitSeverity(msg, messageSeverity);

    // Contextual Technical Details (Objects, Labels)
    logDebugValidationLayerInfo(pCallbackData, messageSeverity);

    printMessageWhitSeverity("================================================================================", messageSeverity);

    return VK_FALSE;
}

// NOLINTBEGIN(*-use-internal-linkage)
VkResult CreateDebugUtilsMessengerEXT(VkInstance instancein, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                      const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger) noexcept {
    // NOLINTNEXTLINE(*-pro-type-reinterpret-cast)
    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instancein, "vkCreateDebugUtilsMessengerEXT"));
    // clang-format off
    if(func != nullptr) [[likely]] {
        return func(instancein, pCreateInfo, pAllocator, pDebugMessenger);
    } else [[unlikely]] {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
    // clang-format on
}

void DestroyDebugUtilsMessengerEXT(VkInstance instancein, VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator) noexcept {
    // NOLINTNEXTLINE(*-pro-type-reinterpret-cast)
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instancein, "vkDestroyDebugUtilsMessengerEXT"));
    if(func != nullptr) [[likely]] { func(instancein, debugMessenger, pAllocator); }
}
// NOLINTEND(*-use-internal-linkage)

// class member functions
Device::Device(Window &window) : window_m{window} {
    VZ_ZONE_SCOPED_NAMED("Device::Constructor");
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createCommandPool();
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer cmdBuffer{};
    vkAllocateCommandBuffers(device_, &allocInfo, &cmdBuffer);
    // Give the profiler init command buffer a debug name so validation messages
    // can identify it if something goes wrong during calibration.
    setObjectName(cmdBuffer, "Profiler Init CommandBuffer");

    profiler.init(
        {.physicalDevice = physicalDevice, .device = device_, .queue = graphicsQueue_, .cmdBuffer = cmdBuffer, .contextName = "GPU"});
    createAllocator();
}

Device::~Device() {
    VZ_ZONE_SCOPED;
#ifndef NDEBUG
    const vnd::AutoTimer timer("Destroying Device");
#endif
    profiler.shutdown();
    vmaDestroyAllocator(allocator);
    vkDestroyCommandPool(device_, commandPool, nullptr);
    vkDestroyDevice(device_, nullptr);

    if(enableValidationLayers) { DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr); }

    vkDestroySurfaceKHR(instance, surface_, nullptr);
    vkDestroyInstance(instance, nullptr);
}

void Device::createInstance() {
    VZ_ZONE_SCOPED_NAMED("Device::createInstance");
    if constexpr(enableValidationLayers) {
        if(!checkValidationLayerSupport()) { throw std::runtime_error("Validation layers requested but not available."); }
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = Vantablade::cmake::project_name.data();
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_4;

    auto extensions = getRequiredExtensions();

#ifdef __APPLE__
    extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = C_UI32T(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

#ifdef __APPLE__
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    VkValidationFeaturesEXT validationFeatures{};
    std::array<VkValidationFeatureEnableEXT, 1> enables = {// VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
                                                           // VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
                                                           VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT};
    /*std::array<VkValidationFeatureDisableEXT, 1> disables = {};*/

    const void *pNextChain = nullptr;

    if constexpr(enableValidationLayers) {
        createInfo.enabledLayerCount = C_UI32T(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        validationFeatures.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
        validationFeatures.enabledValidationFeatureCount = C_UI32T(enables.size());
        validationFeatures.pEnabledValidationFeatures = enables.data();
        /*validationFeatures.disabledValidationFeatureCount = C_UI32T(disables.size());
        validationFeatures.pDisabledValidationFeatures = disables.data();*/

        // Chain: InstanceCreateInfo -> ValidationFeatures -> DebugMessengerCreateInfo
        validationFeatures.pNext = &debugCreateInfo;
        pNextChain = &validationFeatures;
    } else {
        createInfo.enabledLayerCount = 0;
    }
    createInfo.pNext = pNextChain;

    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &instance), "failed to create instance!");

    hasGflwRequiredInstanceExtensions();
}

void Device::pickPhysicalDevice() {
    VZ_ZONE_SCOPED_NAMED("Device::pickPhysicalDevice");
#ifndef NDEBUG
    const vnd::AutoTimer time{"pick Physical Device"};
#endif
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if(deviceCount == 0) { throw std::runtime_error("failed to find GPUs with Vulkan support!"); }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    const auto it = std::ranges::find_if(devices, [this](const VkPhysicalDevice &d) { return isDeviceSuitable(d); });
    if(it != devices.end()) { physicalDevice = *it; }

    if(physicalDevice == VK_NULL_HANDLE) { throw std::runtime_error("failed to find a suitable GPU!"); }

    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    LINFO("Dev count: {}", deviceCount);
    printDeviceInfo(physicalDevice, properties);
}

void Device::createLogicalDevice() {
    VZ_ZONE_SCOPED_NAMED("Device::createLogicalDevice");
    const QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    if(!indices.graphicsFamily.has_value() || !indices.presentFamily.has_value()) {
        throw std::runtime_error("Incomplete queue family indices");
    }

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    const std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    for(const uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // 1. Query supported features into a temporary chain
    DeviceFeatureChain supported{};
    vkGetPhysicalDeviceFeatures2(physicalDevice, &supported.f2);

    // 2. Prepare requested features (zero-initialized by default)
    DeviceFeatureChain features{};

    // 3. Surgical Enablement: Only enable what we actually use or need for compliance
    // Core Features
    features.f2.features.samplerAnisotropy = supported.f2.features.samplerAnisotropy;
    features.f2.features.shaderInt64 = supported.f2.features.shaderInt64;
    features.f2.features.shaderInt16 = supported.f2.features.shaderInt16;
    features.f2.features.fragmentStoresAndAtomics = supported.f2.features.fragmentStoresAndAtomics;
    features.f2.features.vertexPipelineStoresAndAtomics = supported.f2.features.vertexPipelineStoresAndAtomics;

    // Vulkan 1.1
    features.f11.storageBuffer16BitAccess = supported.f11.storageBuffer16BitAccess;

    // Vulkan 1.2
    features.f12.bufferDeviceAddress = supported.f12.bufferDeviceAddress;
    features.f12.shaderInt8 = supported.f12.shaderInt8;
    features.f12.timelineSemaphore = supported.f12.timelineSemaphore;
    features.f12.scalarBlockLayout = supported.f12.scalarBlockLayout;
    features.f12.storageBuffer8BitAccess = supported.f12.storageBuffer8BitAccess;
    features.f12.descriptorIndexing = supported.f12.descriptorIndexing;
    features.f12.runtimeDescriptorArray = supported.f12.runtimeDescriptorArray;
    features.f12.vulkanMemoryModel = supported.f12.vulkanMemoryModel;
    features.f12.vulkanMemoryModelDeviceScope = supported.f12.vulkanMemoryModelDeviceScope;

    // Vulkan 1.3
    features.f13.dynamicRendering = supported.f13.dynamicRendering;
    features.f13.synchronization2 = supported.f13.synchronization2;
    features.f13.maintenance4 = supported.f13.maintenance4;

    // NOTE: robustBufferAccess is explicitly LEFT AS FALSE to avoid conflicts with
    // descriptorBinding*UpdateAfterBind when robustBufferAccessUpdateAfterBind is not supported.

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = &features.f2;

    createInfo.queueCreateInfoCount = C_UI32T(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.enabledExtensionCount = C_UI32T(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    /*if(enableValidationLayers) {
        createInfo.enabledLayerCount = C_UI32T(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }*/

    VK_CHECK(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device_), "failed to create logical device!");

    psetObjectName(instance, "Main Instance");
    psetObjectName(device_, "Main Device");
    psetObjectName(physicalDevice, "Main Physical Device");

    vkGetDeviceQueue(device_, indices.graphicsFamily.value(), 0, &graphicsQueue_);
    vkGetDeviceQueue(device_, indices.presentFamily.value(), 0, &presentQueue_);
    psetObjectName(graphicsQueue_, "Graphics Queue");
    psetObjectName(presentQueue_, "Present Queue");

    psetObjectName(surface_, "Window Surface KHR");
    psetObjectName(debugMessenger, "Debug Utils Messenger");
}

void Device::createCommandPool() {
    VZ_ZONE_SCOPED_NAMED("Device::createCommandPool");
    const QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();
    if(!queueFamilyIndices.graphicsFamily.has_value()) { throw std::runtime_error("failed to find graphics queue family!"); }

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VK_CHECK(vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool), "failed to create command pool!");
    psetObjectName(commandPool, "Command Pool");
}

#ifdef VANTABLADE_PROFILING
static void *VMA_TracyAllocate(void *pUser, VkDeviceSize size, size_t alignment, size_t *pActualSize) {
    void *ptr = std::malloc(size);
    if(ptr) {
        if(pActualSize) *pActualSize = size;
        VZ_MEM_ALLOC(ptr, size);
    }
    return ptr;
}

static void VMA_TracyFree(void *pUser, void *p) {
    VZ_MEM_FREE(p);
    std::free(p);
}

static void *VMA_TracyReallocate(void *pUser, void *p, VkDeviceSize oldSize, VkDeviceSize size, size_t alignment, size_t *pActualSize) {
    VZ_MEM_FREE(p);
    void *ptr = std::realloc(p, size);
    if(ptr) {
        if(pActualSize) *pActualSize = size;
        VZ_MEM_ALLOC(ptr, size);
    }
    return ptr;
}

static VmaAllocatorCallbacks tracyVmaCallbacks = {.pfnAllocate = VMA_TracyAllocate,
                                                  .pfnFree = VMA_TracyFree,
                                                  .pfnReallocate = VMA_TracyReallocate};
#endif

void Device::createAllocator() {
    VZ_ZONE_SCOPED;
    VmaVulkanFunctions vulkanFunctions{};
    vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    vulkanFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
    vulkanFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;

    VmaAllocatorCreateFlags flags = 0;
    // Richiesto se bufferDeviceAddress è abilitato (Vulkan spec + VMA docs).
    if(properties.apiVersion >= VK_API_VERSION_1_2) { flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT; }
    // Opzionale ma raccomandato se VK_EXT_memory_budget è disponibile.
    // flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;

    VmaAllocatorCreateInfo allocatorCreateInfo{};
    allocatorCreateInfo.flags = flags;
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_4;
    allocatorCreateInfo.physicalDevice = physicalDevice;
    allocatorCreateInfo.device = device_;
    allocatorCreateInfo.instance = instance;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
#ifdef VANTABLADE_PROFILING
    allocatorCreateInfo.pAllocator = &tracyVmaCallbacks;
#endif

    VK_CHECK(vmaCreateAllocator(&allocatorCreateInfo, &allocator), "failed to create VMA allocator!");
}

void Device::createSurface() {
    VZ_ZONE_SCOPED;
    window_m.createWindowSurface(instance, &surface_, nullptr);
}

bool Device::isDeviceSuitable(VkPhysicalDevice device) const {
    VZ_ZONE_SCOPED;
    const QueueFamilyIndices indices = findQueueFamilies(device);

    const bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if(extensionsSupported) {
        const SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    // Check for required features using VkPhysicalDeviceFeatures2
    DeviceFeatureChain features{};
    vkGetPhysicalDeviceFeatures2(device, &features.f2);

    return indices.isComplete() && extensionsSupported && swapChainAdequate && (features.f2.features.samplerAnisotropy != 0u);
}

void Device::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
    VZ_ZONE_SCOPED;
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = static_cast<VkDebugUtilsMessageSeverityFlagsEXT>(VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                                                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                                                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT);
    createInfo.messageType = static_cast<VkDebugUtilsMessageTypeFlagsEXT>(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                                                          VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                                                          VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT);
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;  // Optional
}

void Device::setupDebugMessenger() {
    VZ_ZONE_SCOPED;
    if(!enableValidationLayers) { return; }
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerCreateInfo(createInfo);
    VK_CHECK(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger), "failed to set up debug messenger!");
    loadDebugUtilsFunctions();
}

bool Device::checkValidationLayerSupport() {
    VZ_ZONE_SCOPED;
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for(const char *layerName : validationLayers) {
        const bool layerFound = std::ranges::any_of(availableLayers, [layerName](const VkLayerProperties &props) {
            // NOLINTNEXTLINE(*-pro-bounds-array-to-pointer-decay, *-no-array-decay)
            return std::string_view{layerName} == std::string_view{props.layerName};
        });
        if(!layerFound) { return false; }
    }
    return true;
}

// NOLINTNEXTLINE(*-convert-member-functions-to-static)
std::vector<const char *> Device::getRequiredExtensions() const {
    VZ_ZONE_SCOPED;
#ifndef NDEBUG
    const vnd::AutoTimer timer{"get Required Extensions"};
#endif
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    const std::span<const char *> extSpan(glfwExtensions, glfwExtensionCount);
    std::vector<const char *> extensions(extSpan.begin(), extSpan.end());

    if(enableValidationLayers) { extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); }

    return extensions;
}

void Device::hasGflwRequiredInstanceExtensions() {
    VZ_ZONE_SCOPED;
#ifndef NDEBUG
    const vnd::AutoTimer time{"has Gflw Required Instance Extensions"};
#endif
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    std::unordered_set<std::string_view> available;

#ifndef NDEBUG
    std::vector<std::string> availableExtensions;
#endif
    available.reserve(extensionCount);
    for(const auto &[extensionName, specVersion] : extensions) {
#ifndef NDEBUG
        availableExtensions.emplace_back(FORMAT("{} (v. {})", extensionName, specVersion));
#endif
        available.emplace(extensionName);
    }

    const auto requiredExtensions = getRequiredExtensions();
    if(!std::ranges::all_of(requiredExtensions, [&](const auto &required) { return available.contains(required); })) [[unlikely]] {
        throw std::runtime_error("Missing required GLFW extension");
    }
#ifdef NDEBUG
    LINFO("available extensions count: {}", available.size());
    LINFO("required extensions count: {}", requiredExtensions.size());
#else
    LINFO("available extensions:\n  {}", FMT_JOIN(availableExtensions, "\n  "));
    LINFO("required extensions:\n  {}", FMT_JOIN(requiredExtensions, "\n  "));
#endif
}

bool Device::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    VZ_ZONE_SCOPED;
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::unordered_set<std::string_view> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for(const auto &extension : availableExtensions) {
        // NOLINTNEXTLINE(*-pro-bounds-array-to-pointer-decay, *-no-array-decay)
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice phdevice) const {
    VZ_ZONE_SCOPED;
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(phdevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(phdevice, &queueFamilyCount, queueFamilies.data());

    for(const auto &[i, queueFamily] : std::views::enumerate(queueFamilies)) {
        const auto ci = C_UI32T(i);
        if(queueFamily.queueCount > 0 && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0u) { indices.graphicsFamily = ci; }

        VkBool32 presentSupport{VK_FALSE};
        vkGetPhysicalDeviceSurfaceSupportKHR(phdevice, ci, surface_, &presentSupport);

        // NOLINTNEXTLINE(*-implicit-bool-conversion)
        if(queueFamily.queueCount > 0 && presentSupport) { indices.presentFamily = ci; }

        if(indices.isComplete()) { break; }
    }

    return indices;
}

SwapChainSupportDetails Device::querySwapChainSupport(VkPhysicalDevice device) const {
    VZ_ZONE_SCOPED;
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);

    if(formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);

    if(presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, details.presentModes.data());
    }
    return details;
}

VkFormat Device::findSupportedFormat(std::span<const VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const {
    VZ_ZONE_SCOPED;
    for(const VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        // NOLINTBEGIN(*-branch-clone)
        // clang-format off
        if(tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if(tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
        // clang-format on
        // NOLINTEND(*-branch-clone)
    }
    throw std::runtime_error("failed to find supported format!");
}

uint32_t Device::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags propertiesp) const {
    VZ_ZONE_SCOPED;
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    const std::bitset<32> typeBits(typeFilter);
    for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if(typeBits.test(i) && (memProperties.memoryTypes[i].propertyFlags & propertiesp) == propertiesp) { return i; }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void Device::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags propertiesp, VkBuffer &buffer,
                          VmaAllocation &allocation) {
    VZ_ZONE_SCOPED;
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.requiredFlags = propertiesp;
    if((propertiesp & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0u) {
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }

    VK_CHECK(vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr), "failed to create buffer!");
}

VkCommandBuffer Device::beginSingleTimeCommands() {
    VZ_ZONE_SCOPED;
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer{VK_NULL_HANDLE};
    VK_CHECK(vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer), "failed to allocate single-time command buffer!");
    psetObjectName(commandBuffer, "Single Time Command Buffer");

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo), "failed to begin single-time command buffer!");
    pcmdBeginLabel(commandBuffer, "Single Time Commands", DebugColors::Cyan);
    return commandBuffer;
}

void Device::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    VZ_ZONE_SCOPED;
    pcmdEndLabel(commandBuffer);

    VK_CHECK(vkEndCommandBuffer(commandBuffer), "failed to end single-time command buffer!");

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    pqueueBeginLabel(graphicsQueue_, "Transient Upload Submission", DebugColors::Cyan);
    pqueueInsertLabel(graphicsQueue_, "Transient Upload Submission", DebugColors::Cyan);

    VK_CHECK(vkQueueSubmit(graphicsQueue_, 1, &submitInfo, VK_NULL_HANDLE), "failed to submit single-time command buffer!");

    VK_CHECK(vkQueueWaitIdle(graphicsQueue_), "failed to wait for queue idle!");

    pqueueEndLabel(graphicsQueue_);

    vkFreeCommandBuffers(device_, commandPool, 1, &commandBuffer);
}

void Device::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VZ_ZONE_SCOPED;
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    VZ_GPU_ZONE(getProfiler().getContext(), commandBuffer, "Device::copyBuffer");

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;  // Optional
    copyRegion.dstOffset = 0;  // Optional
    copyRegion.size = size;
    pcmdBeginLabel(commandBuffer, "Copy Buffer", DebugColors::Green);
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    pcmdEndLabel(commandBuffer);

    endSingleTimeCommands(commandBuffer);
}

void Device::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) {
    VZ_ZONE_SCOPED;
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    VZ_GPU_ZONE(getProfiler().getContext(), commandBuffer, "Device::copyBufferToImage");

    pcmdBeginLabel(commandBuffer, "Copy Buffer To Image", DebugColors::Yellow);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = layerCount;

    region.imageOffset = {.x = 0, .y = 0, .z = 0};
    region.imageExtent = {.width = width, .height = height, .depth = 1};

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    pcmdEndLabel(commandBuffer);
    endSingleTimeCommands(commandBuffer);
}

void Device::createImageWithInfo(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags propertiesp, VkImage &image,
                                 VmaAllocation &allocation) {
    VZ_ZONE_SCOPED;
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.requiredFlags = propertiesp;
    if((propertiesp & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0u) {
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }

    VK_CHECK(vmaCreateImage(allocator, &imageInfo, &allocInfo, &image, &allocation, nullptr), "failed to create image!");
}

void Device::updateMemoryStats() {
#ifdef VANTABLADE_PROFILING
    std::array<VmaBudget, VK_MAX_MEMORY_HEAPS> budgets{};
    vmaGetHeapBudgets(allocator, budgets.data());

    VmaTotalStatistics stats = {};
    vmaCalculateStatistics(allocator, &stats);

    VkDeviceSize totalBudget = 0;
    for(const auto &budget : budgets) { totalBudget += budget.budget; }

    const float usagePercent = totalBudget == 0
                                   ? 0.0f
                                   : (static_cast<float>(stats.total.statistics.allocationBytes) / static_cast<float>(totalBudget)) *
                                         100.0f;
    VZ_PLOT_FLOAT("VMA Total Memory (MB)", static_cast<float>(stats.total.statistics.allocationBytes / 1024 / 1024));
    VZ_PLOT_FLOAT("VMA Memory Usage %", usagePercent);
    VZ_PLOT_INT("VMA Allocation Count", static_cast<int32_t>(stats.total.statistics.allocationCount));
#endif
}

// clang-format off
// NOLINTEND(*-include-cleaner, *-signed-bitwise, *-easily-swappable-parameters, *-use-anonymous-namespace, *-diagnostic-old-style-cast, *-pro-type-cstyle-cast, *-pro-type-member-init,*-member-init, *-pro-bounds-constant-array-index, *-qualified-auto, *-uppercase-literal-suffix, *-identifier-length, *-magic-numbers, *-diagnostic-missing-designated-field-initializers)
// clang-format on
