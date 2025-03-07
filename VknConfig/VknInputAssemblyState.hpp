#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "VknInfos.hpp"

namespace vkn
{
    class VknInputAssemblyState
    {
    public:
        VknInputAssemblyState();
        VknInputAssemblyState(uint32_t deviceIdx, uint32_t renderpassIdx, uint32_t subpassIdx,
                              VknInfos *infos);
        //~VknInputAssemblyState();

        void setDetails(VkPrimitiveTopology topology, VkBool32 primitiveRestartEnable);
        void fillInputAssemblyStateCreateInfo();

    private:
        uint32_t m_deviceIdx;
        uint32_t m_renderpassIdx;
        uint32_t m_subpassIdx;

        VknInfos *m_infos;

        VkPrimitiveTopology m_topology{VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};
        VkBool32 m_primitiveRestartEnable{VK_FALSE};

        bool m_filled{false};
        bool m_placeholder;
    };
} // namespace vkn