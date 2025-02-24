// I decided to use a Vkn prefix to stand apart from Vk prefixes of the API
// All the structs are in info.hpp

#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

#include "vknInfos.hpp"
#include "VknResult.hpp"
#include "VknPhysicalDevice.hpp"
#include "VknQueueFamily.hpp"

namespace vkn
{
    class VknDevice
    {
    public:
        VknDevice() {}
        VknDevice(VknInfos *infos, VknResultArchive *archive);
        ~VknDevice();
        void addInstance(VkInstance *instance);
        VknResult createDevice();
        void fillDeviceCreateInfo();
        bool vkDeviceCreated() { return m_vkDeviceCreated; }
        void requestQueueFamilyProperties();

        void fillSwapChainCreateInfo(
            VkSurfaceKHR surface, uint32_t imageCount, VkExtent2D dimensions,
            VkSurfaceFormatKHR surfaceFormat = VkSurfaceFormatKHR{VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
            uint32_t numImageArrayLayers = 1, VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
            VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR, VkBool32 clipped = VK_TRUE,
            VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE);
        void addExtensions(std::vector<const char *> ext);
        void createSwapChains();

        VkPhysicalDeviceProperties getPhysicalDeviceProperties()
        {
            return m_physicalDevice.getProperties();
        }
        std::vector<VknQueueFamily> getQueues() { return m_queues; }
        VknQueueFamily getQueue(int idx);
        VkDevice *getVkDevice();
        VknPhysicalDevice *getPhysicalDevice();
        void destroy();

    private:
        VkDevice m_logicalDevice{};
        VknPhysicalDevice m_physicalDevice;
        std::vector<VknQueueFamily> m_queues{};
        VkInstance *m_instance{nullptr};
        VknResultArchive *m_resultArchive{nullptr};
        VknInfos *m_infos{nullptr};

        std::vector<const char *> m_extensions{};
        VkPhysicalDeviceFeatures *m_features{nullptr};
        std::vector<VkSwapchainCreateInfoKHR *> m_swapChainCreateInfos{};
        std::vector<VkSwapchainKHR> m_swapChains{};

        bool m_destroyed{false};
        bool m_instanceAdded{false};
        bool m_vkDeviceCreated{false};

        // Other utilities
        void archiveResult(VknResult res);

        // Init
    };
}