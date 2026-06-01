/*
 * ImGuiLayer — Dear ImGui Vulkan/GLFW backend integration.
 */
// clang-format off
// NOLINTBEGIN(*-include-cleaner, *-signed-bitwise, *-uppercase-literal-suffix, *-avoid-magic-numbers, *-magic-numbers, *-pro-bounds-pointer-arithmetic)
// clang-format on
#include "Vantablade/ImGuiLayer.hpp"
#include "Vantablade/vulkanCheck.hpp"

#include <vulkan/vk_enum_string_helper.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

ImGuiLayer::ImGuiLayer(Device &device, Window &window, VkRenderPass renderPass, uint32_t imageCount)
  : device_m{device}, window_m{window}, renderPass_m{renderPass}, imageCount_m{imageCount} {}

ImGuiLayer::~ImGuiLayer() { onDetach(); }

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void ImGuiLayer::onAttach() {
    if(attached_m) { return; }

    createDescriptorPool();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    // Keyboard / gamepad navigation
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    setupStyle();

    // GLFW backend — install_callbacks=true so ImGui handles keyboard/mouse
    ImGui_ImplGlfw_InitForVulkan(window_m.getGLFWWindow(), true);

    const QueueFamilyIndices indices = device_m.findPhysicalQueueFamilies();
    if(!indices.graphicsFamily.has_value()) { throw std::runtime_error("ImGuiLayer: graphics queue family not found"); }

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = device_m.getInstance();
    initInfo.PhysicalDevice = device_m.getPhysicalDevice();
    initInfo.Device = device_m.device();
    initInfo.QueueFamily = indices.graphicsFamily.value();
    initInfo.Queue = device_m.graphicsQueue();
    initInfo.DescriptorPool = descriptorPool_m;
    initInfo.RenderPass = renderPass_m;
    initInfo.Subpass = 0;
    initInfo.MinImageCount = C_UI32T(SwapChain::MAX_FRAMES_IN_FLIGHT);
    initInfo.ImageCount = imageCount_m;
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    // Use default Vulkan allocator — ImGui manages its own GPU resources.
    initInfo.Allocator = nullptr;
    initInfo.CheckVkResultFn = [](VkResult err) {
        if(err != VK_SUCCESS) [[unlikely]] { throw std::runtime_error(FORMAT("ImGui Vulkan check failed: {}", string_VkResult(err))); }
    };

    ImGui_ImplVulkan_Init(&initInfo);

    // Upload font atlas — handled internally on first NewFrame() in 1.90+,
    // but calling explicitly keeps the init path deterministic.
    ImGui_ImplVulkan_CreateFontsTexture();

    device_m.setObjectName(descriptorPool_m, "ImGui Descriptor Pool");
    attached_m = true;
    LINFO("ImGuiLayer: attached (imageCount={})", imageCount_m);
}

void ImGuiLayer::onDetach() {
    if(!attached_m) { return; }

    vkDeviceWaitIdle(device_m.device());

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    vkDestroyDescriptorPool(device_m.device(), descriptorPool_m, nullptr);
    descriptorPool_m = VK_NULL_HANDLE;
    attached_m = false;

    LINFO("ImGuiLayer: detached");
}

// ---------------------------------------------------------------------------
// Per-frame API
// ---------------------------------------------------------------------------

void ImGuiLayer::begin() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    for(auto &panel : panels_m) { panel->onDraw(); }
}

void ImGuiLayer::end(VkCommandBuffer commandBuffer) {
    ImGui::Render();
    VZ_GPU_ZONE(device_m.getProfiler().getContext(), commandBuffer, "ImGui::Render");
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

// ---------------------------------------------------------------------------
// Swap chain resize
// ---------------------------------------------------------------------------

void ImGuiLayer::onSwapChainRecreated(uint32_t newImageCount) {
    if(!attached_m) { return; }
    imageCount_m = newImageCount;
    ImGui_ImplVulkan_SetMinImageCount(C_UI32T(SwapChain::MAX_FRAMES_IN_FLIGHT));
    LINFO("ImGuiLayer: updated image count to {}", newImageCount);
}

// ---------------------------------------------------------------------------
// Descriptor pool
// ---------------------------------------------------------------------------

void ImGuiLayer::createDescriptorPool() {
    // Generous pool — one per descriptor type as recommended in imgui_impl_vulkan.h.
    constexpr std::array<VkDescriptorPoolSize, 11> poolSizes{{
        {.type = VK_DESCRIPTOR_TYPE_SAMPLER, .descriptorCount = 1000},
        {.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1000},
        {.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, .descriptorCount = 1000},
        {.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .descriptorCount = 1000},
        {.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, .descriptorCount = 1000},
        {.type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, .descriptorCount = 1000},
        {.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1000},
        {.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = 1000},
        {.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, .descriptorCount = 1000},
        {.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, .descriptorCount = 1000},
        {.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, .descriptorCount = 1000},
    }};

    const VkDescriptorPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        // FREE_DESCRIPTOR_SET_BIT is required by the ImGui Vulkan backend.
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 1000,
        .poolSizeCount = C_UI32T(poolSizes.size()),
        .pPoolSizes = poolSizes.data(),
    };

    VK_CHECK(vkCreateDescriptorPool(device_m.device(), &poolInfo, nullptr, &descriptorPool_m), "failed to create ImGui descriptor pool!");
}

// ---------------------------------------------------------------------------
// Style
// ---------------------------------------------------------------------------

void ImGuiLayer::setupStyle() {
    ImGuiIO &io = ImGui::GetIO();

    const auto fontPath = calculateRelativePathToAssets(Vantablade::cmake::project_path(), R"(Fonts\FiraCode\FiraCodeNerdFont-Medium.ttf)");
    LINFO("Loading ImGui font from '{}'", fontPath.string());
    if(!std::filesystem::exists(fontPath)) { LERROR("ImGuiLayer: font file does not exist at '{}'", fontPath.string()); }

    const ImFont *font = io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), 15.0f);
    if(font == nullptr) { LERROR("ImGuiLayer: ImGui failed to load font from '{}'", fontPath.string()); }

    ImGui::StyleColorsDark();

    // NOLINTNEXTLINE(*-identifier-length)
    ImGuiStyle &s = ImGui::GetStyle();
    s.WindowRounding = 5.0f;
    s.FrameRounding = 4.0f;
    s.GrabRounding = 3.0f;
    s.ScrollbarRounding = 4.0f;
    s.TabRounding = 4.0f;
    s.WindowBorderSize = 1.0f;
    s.FrameBorderSize = 0.0f;
    s.IndentSpacing = 14.0f;

    // NOLINTNEXTLINE(*-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay, *-identifier-length)
    ImVec4 *c = s.Colors;
    c[ImGuiCol_WindowBg] = {0.10f, 0.10f, 0.11f, 1.00f};
    c[ImGuiCol_ChildBg] = {0.12f, 0.12f, 0.13f, 1.00f};
    c[ImGuiCol_PopupBg] = {0.10f, 0.10f, 0.11f, 0.94f};
    c[ImGuiCol_FrameBg] = {0.16f, 0.16f, 0.18f, 1.00f};
    c[ImGuiCol_FrameBgHovered] = {0.22f, 0.22f, 0.24f, 1.00f};
    c[ImGuiCol_FrameBgActive] = {0.14f, 0.14f, 0.16f, 1.00f};
    c[ImGuiCol_TitleBg] = {0.10f, 0.10f, 0.11f, 1.00f};
    c[ImGuiCol_TitleBgActive] = {0.18f, 0.18f, 0.20f, 1.00f};
    c[ImGuiCol_TitleBgCollapsed] = {0.10f, 0.10f, 0.11f, 1.00f};
    c[ImGuiCol_MenuBarBg] = {0.14f, 0.14f, 0.16f, 1.00f};
    c[ImGuiCol_ScrollbarBg] = {0.10f, 0.10f, 0.11f, 0.60f};
    c[ImGuiCol_ScrollbarGrab] = {0.30f, 0.30f, 0.34f, 1.00f};
    c[ImGuiCol_ScrollbarGrabHovered] = {0.38f, 0.38f, 0.43f, 1.00f};
    c[ImGuiCol_ScrollbarGrabActive] = {0.22f, 0.22f, 0.25f, 1.00f};
    c[ImGuiCol_CheckMark] = {0.28f, 0.56f, 1.00f, 1.00f};
    c[ImGuiCol_SliderGrab] = {0.28f, 0.56f, 1.00f, 1.00f};
    c[ImGuiCol_SliderGrabActive] = {0.37f, 0.62f, 1.00f, 1.00f};
    c[ImGuiCol_Button] = {0.20f, 0.20f, 0.22f, 1.00f};
    c[ImGuiCol_ButtonHovered] = {0.28f, 0.56f, 1.00f, 1.00f};
    c[ImGuiCol_ButtonActive] = {0.14f, 0.42f, 0.90f, 1.00f};
    c[ImGuiCol_Header] = {0.20f, 0.20f, 0.22f, 1.00f};
    c[ImGuiCol_HeaderHovered] = {0.28f, 0.56f, 1.00f, 0.80f};
    c[ImGuiCol_HeaderActive] = {0.14f, 0.42f, 0.90f, 1.00f};
    c[ImGuiCol_Tab] = {0.14f, 0.14f, 0.16f, 1.00f};
    c[ImGuiCol_TabHovered] = {0.28f, 0.56f, 1.00f, 0.80f};
    c[ImGuiCol_TabActive] = {0.20f, 0.40f, 0.80f, 1.00f};
    c[ImGuiCol_TabUnfocused] = {0.10f, 0.10f, 0.11f, 1.00f};
    c[ImGuiCol_TabUnfocusedActive] = {0.18f, 0.18f, 0.20f, 1.00f};
    c[ImGuiCol_Separator] = {0.28f, 0.28f, 0.32f, 1.00f};
    c[ImGuiCol_ResizeGrip] = {0.28f, 0.56f, 1.00f, 0.20f};
    c[ImGuiCol_ResizeGripHovered] = {0.28f, 0.56f, 1.00f, 0.67f};
    c[ImGuiCol_ResizeGripActive] = {0.28f, 0.56f, 1.00f, 0.95f};
    c[ImGuiCol_TextSelectedBg] = {0.28f, 0.56f, 1.00f, 0.35f};
}
// clang-format off
// NOLINTEND(*-include-cleaner, *-signed-bitwise, *-uppercase-literal-suffix, *-avoid-magic-numbers, *-magic-numbers, *-pro-bounds-pointer-arithmetic)
// clang-format on