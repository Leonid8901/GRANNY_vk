#pragma once
#define NOMINMAX            // Отключение глобальных макросов min/max библиотеки Windows
#define WIN32_LEAN_AND_MEAN // Исключение редко используемых компонентов из заголовков Windows
#include <windows.h>

#define VK_USE_PLATFORM_WIN32_KHR // Включение поддержки расширений WSI для платформы Win32

// Подавление предупреждений статического анализатора MSVC для внешних заголовков SDK
#pragma warning(push)
#pragma warning(disable: 28251) 
#pragma warning(disable: 28252)
#pragma warning(disable: 28301)

#include <vulkan/vulkan.hpp> 
#include <vector>
#include <iostream>

#pragma warning(pop) 

/**
 * @brief Класс инкапсуляции графического контекста Vulkan API 1.3.
 * Реализует паттерн RAII для деструкции объектов логического устройства и системных пулов.
 */
class VulkanContext {
public:
    VulkanContext(HWND hwnd, HINSTANCE hinstance);
    ~VulkanContext();

    /**
     * @brief Запуск полного цикла записи, отправки и презентации графического кадра.
     */
    void drawFrame();

    // Запрет копирования контекста во избежание дублирования дескрипторов Vulkan
    VulkanContext(const VulkanContext&) = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;

private:
    void createInstance();
    void setupDebugMessenger();
    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();

    void createSurface(HWND hwnd, HINSTANCE hinstance);
    void pickPhysicalDevice();
    void createLogicalDevice();
    bool isDeviceSuitable(vk::PhysicalDevice device);
    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createFramebuffers();
    void createCommandPool();

    // Основные дескрипторы инфраструктуры Vulkan API
    vk::Instance m_instance;
    vk::DebugUtilsMessengerEXT m_debugMessenger;
    vk::PhysicalDevice m_physicalDevice;
    vk::Device m_device;
    vk::Queue m_graphicsQueue;
    vk::SurfaceKHR m_surface;

    // Объекты подсистемы Swapchain
    vk::SwapchainKHR m_swapChain;
    std::vector<vk::Image> m_swapChainImages;
    std::vector<vk::ImageView> m_swapChainImageViews;
    vk::Format m_swapChainImageFormat;
    vk::Extent2D m_swapChainExtent;

    // Объекты конвейера отрисовки и синхронизации памяти
    vk::RenderPass m_renderPass;
    std::vector<vk::Framebuffer> m_swapChainFramebuffers;
    vk::CommandPool m_commandPool;
    vk::CommandBuffer m_commandBuffer;

    const std::vector<const char*> m_validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

#ifdef NDEBUG
    const bool m_enableValidationLayers = false;
#else
    const bool m_enableValidationLayers = true;
#endif
};
