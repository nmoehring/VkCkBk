#include <filesystem>

#include "VknPipeline.hpp"
#include <iterator>

namespace vkn
{
    VknPipeline::VknPipeline()
        : m_device{nullptr}, m_infos{nullptr}, m_archive{nullptr}, m_pipeline{nullptr},
          m_deviceIdx{0}, m_renderPassIdx{0}, m_subpassIdx{0}, m_deviceCreated{nullptr}
    {
    }

    VknPipeline::VknPipeline(uint32_t deviceIdx, uint32_t renderPassIdx, uint32_t subpassIdx,
                             VkRenderPass *renderPass, VkPipeline *pipeline,
                             VkDevice *dev, VknInfos *infos, VknResultArchive *archive, const bool *deviceCreated)
        : m_pipeline{pipeline}, m_device{dev}, m_infos{infos}, m_archive{archive},
          m_deviceIdx{deviceIdx}, m_renderPassIdx{renderPassIdx}, m_subpassIdx{subpassIdx},
          m_deviceCreated{deviceCreated}
    {
        m_vertexInputState = VknVertexInputState{deviceIdx, renderPassIdx, subpassIdx, infos};
        m_inputAssemblyState = VknInputAssemblyState{deviceIdx, renderPassIdx, subpassIdx, infos};
        m_multisampleState = VknMultisampleState{deviceIdx, renderPassIdx, subpassIdx, infos};
        m_rasterizationState = VknRasterizationState{deviceIdx, renderPassIdx, subpassIdx, infos};
        m_viewportState = VknViewportState{deviceIdx, renderPassIdx, subpassIdx, infos};
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
                vkDestroyPipeline(*m_device, *m_pipeline, nullptr);
            if (m_pipelineLayoutCreated)
                vkDestroyPipelineLayout(*m_device, m_layout, nullptr);
            for (auto descriptorSetLayout : m_descriptorSetLayouts)
                vkDestroyDescriptorSetLayout(*m_device, descriptorSetLayout, nullptr);
            for (auto module : m_shaderModules)
                vkDestroyShaderModule(*m_device, module, nullptr);
            m_destroyed = true;
            std::cout << "VknPipeline DESTROYED." << std::endl;
        }
    }

    void VknPipeline::addShaderStage(uint32_t shaderIdx,
                                     VknShaderStageType stageType, std::string filename, VkPipelineShaderStageCreateFlags flags)
    {
        if (!m_deviceCreated)
            throw std::runtime_error("Logical device not created before attempting to create shader stage.");
        if (shaderIdx != m_numShaderStages)
            throw std::runtime_error("ShaderIdx passed to addShaderStage is invalid. Should be next idx.");
        m_shaderStages.push_back(VknShaderStage{
            m_deviceIdx, m_renderPassIdx, m_subpassIdx, m_numShaderStages++, m_infos,
            m_archive, m_device});
        m_shaderStages.back().setFilename(filename);
        m_shaderStages.back().setShaderStageType(stageType);
        m_shaderStages.back().setFlags(flags);
    }

    VknShaderStage *VknPipeline::getShaderStage(uint32_t shaderIdx)
    {
        if (shaderIdx >= m_numShaderStages)
            throw std::runtime_error("Shader index out of range.");
        std::list<VknShaderStage>::iterator it = m_shaderStages.begin();
        std::advance(it, shaderIdx);
        return &(*it);
    }

    void VknPipeline::fillPipelineCreateInfo(
        VkPipeline basePipelineHandle, int32_t basePipelineIndex, VkPipelineCreateFlags flags)
    {
        if (m_createInfoFilled)
            throw std::runtime_error("Pipeline create info already filled.");
        m_infos->fillGfxPipelineCreateInfo(
            m_deviceIdx, m_renderPassIdx, m_subpassIdx, &m_layout, basePipelineHandle,
            basePipelineIndex, flags);
        m_createInfoFilled = true;
    }

    void VknPipeline::fillDescriptorSetLayoutCreateInfo(
        VkDescriptorSetLayoutCreateFlags flags)
    {
        m_infos->fillDescriptorSetLayoutCreateInfo(m_bindings, flags);
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
        if (!m_deviceCreated)
            throw std::runtime_error("Logical device not created before attempting to create descriptor set layout.");
        m_descriptorSetLayouts.push_back(VkDescriptorSetLayout{});
        VknResult res{
            vkCreateDescriptorSetLayout(
                *m_device, descriptorSetLayoutCreateInfo,
                nullptr, &m_descriptorSetLayouts.back()),
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
        m_infos->fillPipelineLayoutCreateInfo(m_deviceIdx, m_renderPassIdx, m_subpassIdx,
                                              m_descriptorSetLayouts, m_pushConstantRanges, flags);
    }

    void VknPipeline::createLayout()
    {
        VkPipelineLayoutCreateInfo *layoutCreateInfo = m_infos->getPipelineLayoutCreateInfo(
            m_deviceIdx, m_renderPassIdx, m_subpassIdx);
        vkCreatePipelineLayout(*m_device, layoutCreateInfo, nullptr, &m_layout);
        m_pipelineLayoutCreated = true;
    }

}