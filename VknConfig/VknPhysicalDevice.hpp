#pragma once
#include <vulkan/vulkan.h>

#include "VknResult.hpp"
#include "VknInfos.hpp"

namespace vkn
{
    class VknPhysicalDevice
    {
    public:
        VknPhysicalDevice() {}
        VknPhysicalDevice(VkInstance *instance, VknResultArchive *archive, VknInfos *infos) : m_archive{archive}
        {
            m_infos = infos;
            VknResult res;
            if (!(res = selectPhysicalDevice(instance)).isSuccess())
                throw std::runtime_error(res.toErr("Failed to get physical devices."));
            queryProperties();
        }
        VkPhysicalDevice getDevice() { return m_device; };
        VkPhysicalDeviceProperties getProperties() { return m_properties; };

    private:
        VkPhysicalDevice m_device{};
        VkPhysicalDeviceProperties m_properties{};
        VknResultArchive *m_archive{nullptr};
        VknInfos *m_infos{nullptr};
        VknResult selectPhysicalDevice(VkInstance *instance);
        void queryProperties() { vkGetPhysicalDeviceProperties(m_device, &m_properties); }
    };

}