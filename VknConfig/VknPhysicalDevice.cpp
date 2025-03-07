#include <iostream>

#include "VknPhysicalDevice.hpp"

namespace vkn
{
    VknPhysicalDevice::VknPhysicalDevice()
        : m_archive{nullptr}, m_infos{nullptr}, m_instance{nullptr}, m_instanceCreated{nullptr}
    {
    }

    VknPhysicalDevice::VknPhysicalDevice(VknResultArchive *archive, VknInfos *infos,
                                         const VkInstance *instance, const bool *instanceCreated)
        : m_archive{archive}, m_infos{infos}, m_instance{instance}, m_instanceCreated{instanceCreated}
    {
    }

    VknResult VknPhysicalDevice::selectPhysicalDevice()
    {
        if (!m_instanceCreated)
            throw std::runtime_error("Instance not created before selecting physical device.");

        uint32_t deviceCount{0};
        std::vector<VkPhysicalDevice> devices;

        VknResult res1{vkEnumeratePhysicalDevices(*m_instance, &deviceCount, nullptr),
                       "Enumerate physical devices."};
        m_archive->store(res1);
        if (deviceCount == 0)
            throw std::runtime_error(res1.toErr("No GPU's supporting Vulkan found."));
        else if (deviceCount > 1)
            std::cerr << "Found more than one GPU supporting Vulkan. Selecting device at index 0." << std::endl;

        devices.resize(deviceCount);
        VknResult res2{vkEnumeratePhysicalDevices(*m_instance, &deviceCount, devices.data()),
                       "Enum physical devices and store."};
        m_physicalDevice = devices[0];
        m_selectedPhysicalDevice = true;
        this->queryProperties();

        return res2;
    }

    bool VknPhysicalDevice::getSurfaceSupport(VkSurfaceKHR &surface, uint32_t queueFamilyIdx)
    {
        if (!m_selectedPhysicalDevice)
            throw std::runtime_error("Physical device not selected before getting surface support.");

        VkBool32 presentSupport = false;
        VknResult res{
            vkGetPhysicalDeviceSurfaceSupportKHR(
                m_physicalDevice, queueFamilyIdx, surface, &presentSupport),
            "Get Surface Support"};
        if (!res.isSuccess())
        {
            throw std::runtime_error("Error while getting surface support.");
        }
        m_archive->store(res);
        return presentSupport;
    }

    VkPhysicalDeviceLimits *VknPhysicalDevice::getLimits()
    {
        {
            if (!m_selectedPhysicalDevice)
                throw std::runtime_error("Physical device not selected before getting device limits.");
            return &m_properties.limits;
        }
    }

    void VknPhysicalDevice::queryProperties()
    {
        if (!m_selectedPhysicalDevice)
            throw std::runtime_error("Physical device not selected before getting physical device properties.");
        vkGetPhysicalDeviceProperties(m_physicalDevice, &m_properties);
    }
}