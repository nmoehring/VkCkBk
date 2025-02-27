#include "VknDevice.hpp"

namespace vkn
{
    int VknDevice::s_numDevices{0};

    VknDevice::VknDevice(VknInfos *infos, VknResultArchive *archive, const VkInstance *instance, const bool *instanceCreated)
        : m_infos{infos}, m_resultArchive{archive}, m_instance{instance}, m_instanceCreated{instanceCreated}
    {
        m_physicalDevice = VknPhysicalDevice{m_resultArchive, m_infos, m_instance, m_instanceCreated};
        m_deviceIdx = s_numDevices;
        s_numDevices += 1;
    }

    VknDevice::~VknDevice()
    {
        if (!m_destroyed)
            this->destroy();
    }

    void VknDevice::destroy()
    {
        if (!m_destroyed)
        {
            for (auto renderPass : m_renderPasses)
                renderPass.destroy();
            if (m_vkDeviceCreated)
                vkDestroyDevice(m_logicalDevice, nullptr);
            m_destroyed = true;
        }
    }

    VknPhysicalDevice *VknDevice::getPhysicalDevice()
    {
        return &m_physicalDevice;
    }

    VkDevice *VknDevice::getVkDevice()
    {
        if (!m_vkDeviceCreated)
            throw std::runtime_error("Logical device not created before retrieving it.");
        return &m_logicalDevice;
    }

    VknQueueFamily VknDevice::getQueue(int idx)
    {
        if (!m_queuesRequested)
            throw std::runtime_error("Queue properties not requested before retrieving queue properties.");
        return m_queues[idx];
    }

    void VknDevice::fillDeviceQueuePriorities(uint32_t queueFamilyIdx, std::vector<float> priorities)
    {
        if (!m_queuesRequested)
            throw std::runtime_error("Queue properties not requested before filling queue priorities.");
        m_infos->fillDeviceQueuePriorities(m_deviceIdx, queueFamilyIdx, priorities);
    }

    void VknDevice::fillDeviceQueuePrioritiesDefault()
    {
        if (!m_queuesRequested)
            throw std::runtime_error("Queue properties not requested before filling queue priorities.");
        for (int i = 0; i < m_queues.size(); ++i)
            m_infos->fillDeviceQueuePriorities(m_deviceIdx, i, std::vector<float>(m_queues[i].getNumSelected(), 1.0f));
    }

    void VknDevice::archiveResult(VknResult res)
    {
        m_resultArchive->store(res);
    }

    void VknDevice::fillDeviceCreateInfo()
    {
        m_infos->fillDeviceExtensionNames(m_deviceIdx, m_extensions, m_extensionsSize);
        m_infos->fillDeviceFeatures(m_features);
        m_infos->fillDeviceCreateInfo(m_deviceIdx);
    }

    VknRenderPass *VknDevice::getRenderPass(uint32_t renderPassIdx)
    {
        if (renderPassIdx >= m_renderPasses.size())
            throw std::runtime_error("This renderpass not added before attempting to get() it.");
        return &(m_renderPasses[renderPassIdx]);
    }

    void VknDevice::addExtensions(const char *ext[], uint32_t size)
    {
        m_extensions = ext;
        m_extensionsSize = size;
    }

    void VknDevice::requestQueueFamilyProperties()
    {
        if (!(m_physicalDevice.getPhysicalDeviceSelected()))
            throw std::runtime_error("Physical device not selected before requesting queue properties.");
        uint32_t propertyCount{0};
        vkGetPhysicalDeviceQueueFamilyProperties(
            *(m_physicalDevice.getVkPhysicalDevice()), &propertyCount, VK_NULL_HANDLE);
        if (propertyCount == 0)
            throw std::runtime_error("No available queue families found.");
        std::vector<VkQueueFamilyProperties> queues;
        queues.resize(propertyCount);

        vkGetPhysicalDeviceQueueFamilyProperties(
            *(m_physicalDevice.getVkPhysicalDevice()),
            &propertyCount,
            queues.data());
        if (queues.size() == 0)
            throw std::runtime_error("Error getting queue family properties.");
        for (auto props : queues)
            m_queues.push_back(VknQueueFamily(props));
        m_queuesRequested = true;
    }

    VknResult VknDevice::createDevice()
    {
        if (!(m_physicalDevice.getPhysicalDeviceSelected()))
            throw std::runtime_error("Physical device not selected before creating logical device.");
        VknResult res{
            vkCreateDevice(
                *(m_physicalDevice.getVkPhysicalDevice()),
                m_infos->getDeviceCreateInfo(m_deviceIdx),
                nullptr,
                &m_logicalDevice),
            "Create device"};
        if (!(res.isSuccess()))
            throw std::runtime_error(res.toErr("Error creating device."));
        m_resultArchive->store(res);
        m_vkDeviceCreated = true;
        return res;
    }

    void VknDevice::fillSwapChainCreateInfo(
        VkSurfaceKHR surface, uint32_t imageCount, VkExtent2D dimensions,
        VkSurfaceFormatKHR surfaceFormat, uint32_t numImageArrayLayers, VkImageUsageFlags usage,
        VkSharingMode sharingMode, VkSurfaceTransformFlagBitsKHR preTransform,
        VkCompositeAlphaFlagBitsKHR compositeAlpha, VkPresentModeKHR presentMode, VkBool32 clipped,
        VkSwapchainKHR oldSwapchain)
    {
        m_swapChainCreateInfos.push_back(m_infos->fillSwapChainCreateInfo(m_deviceIdx, surface, imageCount, dimensions, surfaceFormat,
                                                                          numImageArrayLayers, usage, sharingMode, preTransform,
                                                                          compositeAlpha, presentMode, clipped, oldSwapchain));
    }

    void VknDevice::createSwapChains()
    {
        if (!m_vkDeviceCreated)
            throw std::runtime_error("VkDevice not created before creating swapchains.");
        for (auto swapchainInfo : m_swapChainCreateInfos)
        {
            m_swapChains.push_back(VkSwapchainKHR{});
            VknResult res{
                vkCreateSwapchainKHR(m_logicalDevice, swapchainInfo, nullptr, &(m_swapChains.back())),
                "Create swapchain"};
            if (!res.isSuccess())
                throw std::runtime_error(res.toErr("Error creating swapchain."));
            m_resultArchive->store(res);
        }
    }

    uint32_t VknDevice::addRenderPass()
    {
        m_renderPasses.push_back(VknRenderPass(m_deviceIdx, m_renderPasses.size(), m_infos, m_resultArchive,
                                               &m_logicalDevice, &m_vkDeviceCreated));
        return m_renderPasses.size();
    }
} // namespace vkn