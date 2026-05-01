// NOLINTBEGIN(*-include-cleaner)
#include <CLI/CLI.hpp>
#include <Vantablade/vantablade.hpp>

// NOLINTNEXTLINE(bugprone-exception-escape)
int main(int argc, const char **argv) {
    const auto version = FORMAT("{} version {} git sha {}", Vantablade::cmake::project_name, Vantablade::cmake::project_version,
                                Vantablade::cmake::git_short_sha);
    CLI::App app{version};
    try {
        app.set_version_flag("--version, -v", version);
        app.parse(argc, argv);

        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        GLFWwindow *window = glfwCreateWindow(800, 600, "Vulkan GLFW", nullptr, nullptr);

        uint32_t extensionCount = 0;
        const char **extensions = glfwGetRequiredInstanceExtensions(&extensionCount);

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.enabledExtensionCount = extensionCount;
        createInfo.ppEnabledExtensionNames = extensions;

        VkInstance instance;
        if(vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            std::cerr << "Instance creation failed\n";
            return -1;
        }

        VkSurfaceKHR surface;
        if(glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            std::cerr << "Surface creation failed\n";
            return -1;
        }

        while(!glfwWindowShouldClose(window)) { glfwPollEvents(); }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
        glfwDestroyWindow(window);
        glfwTerminate();
    } catch(const CLI::ParseError &e) { return app.exit(e); } catch(const std::exception &e) {
        // Handle any other types of exceptions
        LERROR("Unhandled exception in main: {}", e.what());
    }
}
// NOLINTEND(*-include-cleaner)