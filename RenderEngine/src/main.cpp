
// ===============================================================
// main.cpp
// ===============================================================
// Standard headers
#include <array>
#include <cstdint>
#include <iostream>
// #include <string>
// #include <vector>
// #include <cassert>

// Third-party
// Windowing (no Vulkan included here)
#include <GLFW/glfw3.h>

// Vulkan loader (defines VK_NO_PROTOTYPES and includes vulkan.h safely)
#define VK_NO_PROTOTYPES
#include <volk/volk.h>
#include <vulkan/vulkan_core.h>
// ===============================================================

uint32_t selectBestVersion()
{
    uint32_t systemVersion = 0;

    // Detect the maximum Vulkan version supported by the system/driver
    if (vkEnumerateInstanceVersion(&systemVersion) != VK_SUCCESS) {
        systemVersion = VK_API_VERSION_1_0;
    }

    // Define version priorities for the engine
    const std::array<uint32_t, 4> preferredVersions = {{VK_API_VERSION_1_3,
                                                        VK_API_VERSION_1_2,
                                                        VK_API_VERSION_1_1,
                                                        VK_API_VERSION_1_0}};

    // Select the highest version that the system supports
    for (uint32_t version : preferredVersions) {
        if (systemVersion >= version) {
            return version;
        }
    }

    return VK_API_VERSION_1_0; // Fallback
}

void selectAndPrintBestVersion()
{
    std::cout << "=== Vulkan Version Selection ===" << std::endl;

    // Detect system capabilities
    uint32_t systemVersion = 0;
    if (vkEnumerateInstanceVersion(&systemVersion) != VK_SUCCESS) {
        systemVersion = VK_API_VERSION_1_0;
        std::cout << "System detection failed, assuming Vulkan 1.0" << std::endl;
    } else {
        std::cout << "System supports up to: "
                  << VK_VERSION_MAJOR(systemVersion) << "."
                  << VK_VERSION_MINOR(systemVersion) << "."
                  << VK_VERSION_PATCH(systemVersion) << std::endl;
    }

    // Determine the best version for the engine
    uint32_t selectedVersion = selectBestVersion();

    std::cout << "Selected for engine: "
              << VK_VERSION_MAJOR(selectedVersion) << "."
              << VK_VERSION_MINOR(selectedVersion) << "."
              << VK_VERSION_PATCH(selectedVersion) << std::endl;

    // Compatibility feedback
    if (selectedVersion >= VK_API_VERSION_1_3) {
        std::cout << "Full modern Vulkan features available!" << std::endl;
    } else if (selectedVersion >= VK_API_VERSION_1_2) {
        std::cout << "Good feature support" << std::endl;
    } else {
        std::cout << "Limited to basic features" << std::endl;
    }
}

int main()
{
    std::cout << "Hello CG! Hey NoRender Engine" << std::endl;

    // Initialize volk (loads initial function pointers for instance creation)
    if (volkInitialize() != VK_SUCCESS) {
        std::cerr << "Failed to initialize volk!" << std::endl;
        return -1;
    }

    selectAndPrintBestVersion();

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = selectBestVersion();

    // 启用 Validation Layers（调试必备)
    // 在创建 VkInstance 前
    // const std::vector<const char*> validationLayers = {
    // "VK_LAYER_KHRONOS_validation"
    // };

    // #ifdef NDEBUG
    // const bool enableValidationLayers = false;
    // #else
    // const bool enableValidationLayers = true;
    // #endif

    // 创建实例时
    // VkInstanceCreateInfo createInfo{};
    // createInfo.enabledLayerCount = enableValidationLayers ? static_cast<uint32_t>(validationLayers.size()) : 0;
    // createInfo.ppEnabledLayerNames = enableValidationLayers ? validationLayers.data() : nullptr;

    return 0;
}