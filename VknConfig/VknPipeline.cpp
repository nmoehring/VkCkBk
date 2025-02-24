#include <filesystem>

#include "VknPipeline.hpp"
#include "VknInfos.hpp"
#include "../Utilities/Utilities.hpp"

namespace vkn
{

    VknPipeline::VknPipeline(VkRenderPass *renderPass, VkSubpassDescription *subpass, VkPipeline *pipeline,
                             VknDevice *dev, VknInfos *infos, VknResultArchive *archive, uint32_t index)
        : m_subpass{subpass}, m_pipeline{pipeline}, m_device{dev},
          m_infos{infos}, m_archive{archive}, m_index{index}
    {
        m_renderPass = renderPass;
        m_attachmentReferences = m_infos->getAttachmentReferences(m_index);
        m_preserveAttachments = m_infos->getPreserveAttachments(m_index);
    }

    VknPipeline::~VknPipeline()
    {
        if (!m_destroyed)
            this->destroy();
    }

    void VknPipeline::destroy()
    {
        if (!m_destroyed)
        {
            if (m_pipelineCreated)
                vkDestroyPipeline(*(m_device->getVkDevice()), *m_pipeline, nullptr);
            if (m_pipelineLayoutCreated)
                vkDestroyPipelineLayout(*(m_device->getVkDevice()), m_layout, nullptr);
            for (auto descriptorSetLayout : m_descriptorSetLayouts)
                vkDestroyDescriptorSetLayout(*(m_device->getVkDevice()), descriptorSetLayout, nullptr);
            for (auto module : m_shaderModules)
                vkDestroyShaderModule(*(m_device->getVkDevice()), module, nullptr);
            m_destroyed = true;
        }
    }

    void VknPipeline::fillPipelineCreateInfo(
        VkPipeline basePipelineHandle, int32_t basePipelineIndex, VkPipelineCreateFlags flags,
        VkPipelineVertexInputStateCreateInfo *pVertexInputState,
        VkPipelineInputAssemblyStateCreateInfo *pInputAssemblyState,
        VkPipelineTessellationStateCreateInfo *pTessellationState,
        VkPipelineViewportStateCreateInfo *pViewportState,
        VkPipelineRasterizationStateCreateInfo *pRasterizationState,
        VkPipelineMultisampleStateCreateInfo *pMultisampleState,
        VkPipelineDepthStencilStateCreateInfo *pDepthStencilState,
        VkPipelineColorBlendStateCreateInfo *pColorBlendState,
        VkPipelineDynamicStateCreateInfo *pDynamicState)
    {
        m_createInfo = m_infos->fillGfxPipelineCreateInfo(m_shaderStageInfos, &m_layout, m_renderPass, m_index,
                                                          basePipelineHandle, basePipelineIndex, flags, pVertexInputState,
                                                          pInputAssemblyState, pTessellationState, pViewportState,
                                                          pRasterizationState, pMultisampleState, pDepthStencilState, pColorBlendState, pDynamicState);
    }

    VkDescriptorSetLayoutCreateInfo *VknPipeline::fillDescriptorSetLayoutCreateInfo(
        VkDescriptorSetLayoutCreateFlags flags)
    {
        return m_infos->fillDescriptorSetLayoutCreateInfo(m_bindings, flags);
    }

    void VknPipeline::createDescriptorSetLayoutBinding(
        uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount,
        VkShaderStageFlags stageFlags, const VkSampler *pImmutableSamplers)
    {
        m_bindings.push_back(VkDescriptorSetLayoutBinding{});
        m_bindings.back().binding = binding;
        m_bindings.back().descriptorType = descriptorType;
        m_bindings.back().descriptorCount = descriptorCount;
        m_bindings.back().stageFlags = stageFlags;
        m_bindings.back().pImmutableSamplers = pImmutableSamplers;
    }

    void VknPipeline::createDescriptorSetLayout(VkDescriptorSetLayoutCreateInfo *descriptorSetLayoutCreateInfo)
    {
        m_descriptorSetLayouts.push_back(VkDescriptorSetLayout{});
        VknResult res{
            vkCreateDescriptorSetLayout(
                *(m_device->getVkDevice()), descriptorSetLayoutCreateInfo,
                nullptr, &(m_descriptorSetLayouts.back())),
            "Create descriptor set layout."};
        if (!res.isSuccess())
            throw std::runtime_error(res.toErr("Error creating descriptor set layout."));
        m_archive->store(res);
    }

    void VknPipeline::createPushConstantRange(VkShaderStageFlags stageFlags, uint32_t offset,
                                              uint32_t size)
    {
        m_pushConstantRanges.push_back(VkPushConstantRange{});
        m_pushConstantRanges.back().stageFlags = stageFlags;
        m_pushConstantRanges.back().offset = offset;
        m_pushConstantRanges.back().size = size;
    }

    void VknPipeline::fillPipelineLayoutCreateInfo(VkPipelineLayoutCreateFlags flags)
    {
        m_layoutCreateInfo = m_infos->fillPipelineLayoutCreateInfo(m_descriptorSetLayouts, m_pushConstantRanges, flags);
    }

    void VknPipeline::createLayout()
    {
        vkCreatePipelineLayout(*(m_device->getVkDevice()), m_layoutCreateInfo, nullptr, &m_layout);
        m_pipelineLayoutCreated = true;
    }

    int VknPipeline::createShaderModule(const std::string filename)
    {
        std::filesystem::path shaderDir = std::filesystem::current_path() / "resources" / "shaders";
        std::vector<char> code{CCUtilities::readBinaryFile(shaderDir / filename)};
        auto createInfo{m_infos->fillShaderModuleCreateInfo(code)};
        VkShaderModule shaderModule;
        if (vkCreateShaderModule(*(m_device->getVkDevice()), createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create shader module!");
        }
        m_shaderModules.push_back(shaderModule);
        return m_shaderModules.size() - 1;
    }

    int VknPipeline::createShaderStage(ShaderStage shaderStage, std::string filename)
    {
        int module_idx{this->createShaderModule(filename)};
        VkShaderStageFlagBits shaderStageFlagBits{};
        switch (shaderStage)
        {
        case VKN_VERTEX_STAGE:
            shaderStageFlagBits = VK_SHADER_STAGE_VERTEX_BIT;
            break;
        case VKN_FRAGMENT_STAGE:
            shaderStageFlagBits = VK_SHADER_STAGE_FRAGMENT_BIT;
            break;
        default:
            throw std::runtime_error("Shader stage not recognized.");
        }
        m_shaderStageInfos.push_back(
            m_infos->fillShaderStageCreateInfo(m_shaderModules[module_idx], shaderStageFlagBits));
        return m_shaderStageInfos.size() - 1;
    }

    void VknPipeline::setVertexInput()
    {
        m_infos->fillVertexInputStateCreateInfo(m_index);
    }

    void VknPipeline::fillVertexBindingDescription(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate)
    {
        m_vertexBindingDescriptions.push_back(m_infos->fillVertexInputBindingDescription(m_index, binding, stride, inputRate));
    }
    void VknPipeline::fillVertexAttributeDescription(uint32_t binding, uint32_t location, VkFormat format, uint32_t offset)
    {
        m_infos->fillVertexInputAttributeDescription(m_index, binding, location, format, offset);
    }
}