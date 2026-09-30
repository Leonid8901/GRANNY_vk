#include "VulkanContext.h"
#include <stdexcept>
#include <cstring>
#include <limits>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    std::cerr << "[Vulkan Validation]: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

VulkanContext::VulkanContext(HWND hwnd, HINSTANCE hinstance) {
    createInstance();
    setupDebugMessenger();
    createSurface(hwnd, hinstance);
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createFramebuffers();
    createCommandPool();
}

VulkanContext::~VulkanContext() {
    if (m_device) {
        if (m_commandPool) {
            m_device.destroyCommandPool(m_commandPool);
        }
        for (auto framebuffer : m_swapChainFramebuffers) {
            m_device.destroyFramebuffer(framebuffer);
        }
        m_swapChainFramebuffers.clear();

        if (m_renderPass) {
            m_device.destroyRenderPass(m_renderPass);
        }
        for (auto imageView : m_swapChainImageViews) {
            m_device.destroyImageView(imageView);
        }
        m_swapChainImageViews.clear();

        if (m_swapChain) {
            m_device.destroySwapchainKHR(m_swapChain);
        }
        m_device.destroy();
    }

    if (m_instance && m_surface) {
        m_instance.destroySurfaceKHR(m_surface);
    }
    if (m_enableValidationLayers && m_debugMessenger) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)m_instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(m_instance, m_debugMessenger, nullptr);
        }
    }
    if (m_instance) {
        m_instance.destroy();
    }
}

bool VulkanContext::checkValidationLayerSupport() {
    std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();
    for (const char* layerName : m_validationLayers) {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) return false;
    }
    return true;
}

std::vector<const char*> VulkanContext::getRequiredExtensions() {
    std::vector<const char*> extensions;
    extensions.push_back("VK_KHR_surface");
    extensions.push_back("VK_KHR_win32_surface");
    if (m_enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return extensions;
}

void VulkanContext::createInstance() {
    if (m_enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("Слои валидации запрошены, но не доступны в системе!");
    }

    vk::ApplicationInfo appInfo("Granny_Scratch", VK_MAKE_VERSION(1, 0, 0), "No_Engine", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_3);
    auto extensions = getRequiredExtensions();

    vk::InstanceCreateInfo createInfo(
        vk::InstanceCreateFlags(), &appInfo,
        0, nullptr,
        static_cast<uint32_t>(extensions.size()), extensions.data()
    );

    if (m_enableValidationLayers) {
        createInfo.setEnabledLayerCount(static_cast<uint32_t>(m_validationLayers.size()));
        createInfo.setPpEnabledLayerNames(m_validationLayers.data());
    }

    m_instance = vk::createInstance(createInfo);
    std::cout << "[VulkanContext] VkInstance успешно создан! Версия Vulkan 1.3\n";
}

void VulkanContext::setupDebugMessenger() {
    if (!m_enableValidationLayers) return;

    vk::DebugUtilsMessengerCreateInfoEXT createInfo(
        vk::DebugUtilsMessengerCreateFlagsEXT(),
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        reinterpret_cast<vk::PFN_DebugUtilsMessengerCallbackEXT>(debugCallback)
    );

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)m_instance.getProcAddr("vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        VkDebugUtilsMessengerEXT rawMessenger;
        VkDebugUtilsMessengerCreateInfoEXT rawCreateInfo = createInfo;
        if (func(m_instance, &rawCreateInfo, nullptr, &rawMessenger) == VK_SUCCESS) {
            m_debugMessenger = rawMessenger;
            std::cout << "[VulkanContext] Слои валидации и отладчик успешно подключены.\n";
        }
    }
    else {
        throw std::runtime_error("Не удалось настроить отладчик Vulkan.");
    }
}

void VulkanContext::createSurface(HWND hwnd, HINSTANCE hinstance) {
    vk::Win32SurfaceCreateInfoKHR createInfo(vk::Win32SurfaceCreateFlagsKHR(), hinstance, hwnd);
    m_surface = m_instance.createWin32SurfaceKHR(createInfo);
    std::cout << "[VulkanContext] Поверхность VkSurface успешно создана.\n";
}

void VulkanContext::pickPhysicalDevice() {
    std::vector<vk::PhysicalDevice> devices = m_instance.enumeratePhysicalDevices();
    if (devices.empty()) throw std::runtime_error("Отсутствует GPU с поддержкой Vulkan API.");

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            m_physicalDevice = device;
            break;
        }
    }
    if (!m_physicalDevice && !devices.empty()) m_physicalDevice = devices[0];
    if (!m_physicalDevice) throw std::runtime_error("Не найдена подходящая видеокарта.");

    vk::PhysicalDeviceProperties props = m_physicalDevice.getProperties();
    std::cout << "[Render] Активное устройство: " << props.deviceName << "\n";
}

bool VulkanContext::isDeviceSuitable(vk::PhysicalDevice device) {
    vk::PhysicalDeviceProperties properties = device.getProperties();
    return properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
}

void VulkanContext::createLogicalDevice() {
    float queuePriority = 1.0f;
    vk::DeviceQueueCreateInfo queueCreateInfo(vk::DeviceQueueCreateFlags(), 0, 1, &queuePriority);
    vk::PhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    vk::DeviceCreateInfo createInfo(
        vk::DeviceCreateFlags(), 1, &queueCreateInfo,
        0, nullptr, static_cast<uint32_t>(deviceExtensions.size()), deviceExtensions.data(),
        &deviceFeatures
    );

    m_device = m_physicalDevice.createDevice(createInfo);
    m_graphicsQueue = m_device.getQueue(0, 0);
    std::cout << "[VulkanContext] Логическое устройство и Графическая Очередь созданы!\n";
}

void VulkanContext::createSwapChain() {
    m_swapChainImageFormat = vk::Format::eB8G8R8A8Unorm;
    m_swapChainExtent = vk::Extent2D(1280, 720);
    uint32_t imageCount = 2;

    vk::SwapchainCreateInfoKHR createInfo(
        vk::SwapchainCreateFlagsKHR(), m_surface, imageCount, m_swapChainImageFormat,
        vk::ColorSpaceKHR::eSrgbNonlinear, m_swapChainExtent, 1,
        vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive,
        0, nullptr, vk::SurfaceTransformFlagBitsKHR::eIdentity,
        vk::CompositeAlphaFlagBitsKHR::eOpaque, vk::PresentModeKHR::eFifo, VK_TRUE, nullptr
    );

    m_swapChain = m_device.createSwapchainKHR(createInfo);
    m_swapChainImages = m_device.getSwapchainImagesKHR(m_swapChain);
    std::cout << "[VulkanContext] Свопчейн успешно создан (" << m_swapChainImages.size() << " буфера).\n";
}

void VulkanContext::createImageViews() {
    m_swapChainImageViews.resize(m_swapChainImages.size());
    for (size_t i = 0; i < m_swapChainImages.size(); i++) {
        vk::ImageViewCreateInfo createInfo(
            vk::ImageViewCreateFlags(), m_swapChainImages[i], vk::ImageViewType::e2D, m_swapChainImageFormat,
            vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
        );
        m_swapChainImageViews[i] = m_device.createImageView(createInfo);
    }
    std::cout << "[VulkanContext] Создано " << m_swapChainImageViews.size() << " окон доступа VkImageView.\n";
}

void VulkanContext::createRenderPass() {
    vk::AttachmentDescription colorAttachment(
        vk::AttachmentDescriptionFlags(),
        m_swapChainImageFormat,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::ePresentSrcKHR
    );

    vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription subpass(
        vk::SubpassDescriptionFlags(),
        vk::PipelineBindPoint::eGraphics,
        0, nullptr,
        1, &colorAttachmentRef
    );

    vk::RenderPassCreateInfo renderPassInfo(
        vk::RenderPassCreateFlags(),
        1, &colorAttachment,
        1, &subpass
    );

    m_renderPass = m_device.createRenderPass(renderPassInfo);
    std::cout << "[VulkanContext] Паспорт отрисовки VkRenderPass успешно создан.\n";
}

void VulkanContext::createFramebuffers() {
    m_swapChainFramebuffers.resize(m_swapChainImageViews.size());

    for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {
        vk::ImageView attachments[] = {
            m_swapChainImageViews[i]
        };

        vk::FramebufferCreateInfo framebufferInfo(
            vk::FramebufferCreateFlags(),
            m_renderPass,
            1, attachments,
            m_swapChainExtent.width,
            m_swapChainExtent.height,
 1 );
        m_swapChainFramebuffers[i] = m_device.createFramebuffer(framebufferInfo);
    }
    std::cout << "[VulkanContext] Фреймбуферы VkFramebuffer успешно созданы.\n";
}
void VulkanContext::createCommandPool() {
    vk::CommandPoolCreateInfo poolInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, 0);
    m_commandPool = m_device.createCommandPool(poolInfo);
    vk::CommandBufferAllocateInfo allocInfo(m_commandPool, vk::CommandBufferLevel::ePrimary, 1);
    m_commandBuffer = m_device.allocateCommandBuffers(allocInfo)[0];
    std::cout << "[VulkanContext] Пул и Командный буфер успешно выделены в видеопамяти AMD.\n";
}
void VulkanContext::drawFrame() {
    auto resultValue = m_device.acquireNextImageKHR(m_swapChain, UINT64_MAX, nullptr, nullptr);
    if (resultValue.result == vk::Result::eErrorOutOfDateKHR) return;
    uint32_t imageIndex = resultValue.value;
    m_commandBuffer.reset();
    m_commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    vk::ClearValue clearColor;
    clearColor.color = vk::ClearColorValue(std::array<float, 4>{0.02f, 0.04f, 0.15f, 1.0f});
    vk::RenderPassBeginInfo renderPassInfo(
        m_renderPass, m_swapChainFramebuffers[imageIndex],
        vk::Rect2D(vk::Offset2D(0, 0), m_swapChainExtent), 1, &clearColor
    );
    m_commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    m_commandBuffer.endRenderPass();
    m_commandBuffer.end();
    vk::SubmitInfo submitInfo(0, nullptr, nullptr, 1, &m_commandBuffer, 0, nullptr);
    m_graphicsQueue.submit(submitInfo);
    m_device.waitIdle();
    vk::PresentInfoKHR presentInfo(0, nullptr, 1, &m_swapChain, &imageIndex);
    try {
        m_graphicsQueue.presentKHR(presentInfo);
    }
    catch (const vk::OutOfDateKHRError&) {
        return;
    }
}