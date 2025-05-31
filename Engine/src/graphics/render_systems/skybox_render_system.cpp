#include "graphics/render_systems/skybox_render_system.hpp"

#include "core/memory.hpp"
#include "core/diagnostics.hpp"
#include "core/constants.hpp"

#include <stdexcept>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace PXTEngine {

    // No push constants needed for skybox, as it doesn't transform based on a model matrix
    // It's usually rendered at the camera's position.

    SkyboxRenderSystem::SkyboxRenderSystem(
        Context& context,
        Shared<DescriptorAllocatorGrowable> descriptorAllocator,
        DescriptorSetLayout& globalSetLayout,
        VkRenderPass renderPass)
        : m_context(context),
        m_descriptorAllocator(descriptorAllocator) {

        // TODO: remove from here
        std::array<std::string, 6> paths;
        paths[CubeFace::BACK] = TEXTURES_PATH + "skybox/bluecloud_bk.jpg";
        paths[CubeFace::FRONT] = TEXTURES_PATH + "skybox/bluecloud_ft.jpg";
        paths[CubeFace::LEFT] = TEXTURES_PATH + "skybox/bluecloud_lf.jpg";
        paths[CubeFace::RIGHT] = TEXTURES_PATH + "skybox/bluecloud_rt.jpg";
        paths[CubeFace::TOP] = TEXTURES_PATH + "skybox/bluecloud_dn.jpg";
        paths[CubeFace::BOTTOM] = TEXTURES_PATH + "skybox/bluecloud_up.jpg";

        m_skybox = createUnique<Skybox>(
            m_context,
            paths
        );

        createDescriptorSets();
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    SkyboxRenderSystem::~SkyboxRenderSystem() {
        vkDestroyPipelineLayout(m_context.getDevice(), m_pipelineLayout, nullptr);
    }

    void SkyboxRenderSystem::createDescriptorSets() {
        m_skyboxDescriptorSetLayout = DescriptorSetLayout::Builder(m_context)
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

        m_descriptorAllocator->allocate(m_skyboxDescriptorSetLayout->getDescriptorSetLayout(), m_skyboxDescriptorSet);

        // Get the VkDescriptorImageInfo from the Skybox object
        VkDescriptorImageInfo skyboxImageInfo = m_skybox->getDescriptorImageInfo();

        DescriptorWriter(m_context, *m_skyboxDescriptorSetLayout)
            .writeImage(0, &skyboxImageInfo)
            .updateSet(m_skyboxDescriptorSet);
    }

    void SkyboxRenderSystem::createPipelineLayout(DescriptorSetLayout& globalSetLayout) {
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
            globalSetLayout.getDescriptorSetLayout(),
            m_skyboxDescriptorSetLayout->getDescriptorSetLayout()
        };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0; // No push constants for skybox
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(m_context.getDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create skybox pipeline layout!");
        }
    }

    void SkyboxRenderSystem::createPipeline(VkRenderPass renderPass) {
        PXT_ASSERT(m_pipelineLayout != nullptr, "Cannot create skybox pipeline before pipelineLayout");

        RasterizationPipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_pipelineLayout;

        // Skybox-specific pipeline settings:
        // No depth writing, but depth testing is enabled (equal or less than) to render behind opaque objects
        pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
        pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL; // Draw skybox behind everything

        pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        pipelineConfig.rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        // Use a simpler vertex input state as we won't have complex vertex attributes
        pipelineConfig.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.bindingDescriptions.clear();


        const std::vector<std::pair<VkShaderStageFlagBits, std::string>>& shaderFilePaths = {
            {VK_SHADER_STAGE_VERTEX_BIT, SPV_SHADERS_PATH + "skybox.vert.spv"},
            {VK_SHADER_STAGE_FRAGMENT_BIT, SPV_SHADERS_PATH + "skybox.frag.spv"}
        };

        m_pipeline = createUnique<Pipeline>(
            m_context,
            shaderFilePaths,
            pipelineConfig
        ); 
    }

    void SkyboxRenderSystem::render(FrameInfo& frameInfo) {
        m_pipeline->bind(frameInfo.commandBuffer);

        std::array<VkDescriptorSet, 2> descriptorSets = { frameInfo.globalDescriptorSet, m_skyboxDescriptorSet };

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipelineLayout,
            0,
            static_cast<uint32_t>(descriptorSets.size()),
            descriptorSets.data(),
            0,
            nullptr
        );

        // Draw 36 vertices (12 triangles) for a cube
        vkCmdDraw(frameInfo.commandBuffer, 36, 1, 0, 0);
    }

} 