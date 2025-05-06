#include "graphics/render_systems/debug_render_system.hpp"

#include "core/memory.hpp"
#include "core/diagnostics.hpp"
#include "core/constants.hpp"
#include "graphics/resources/vk_mesh.hpp"
#include "scene/ecs/entity.hpp"

#include <stdexcept>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace PXTEngine {

    struct DebugPushConstantData {
        glm::mat4 modelMatrix{1.f};
        glm::mat4 normalMatrix{1.f};
		glm::vec4 color{ 1.f };
        uint32_t enableWireframe{0};
		uint32_t enableNormals{0};
		int textureIndex = 0;
		int normalMapIndex = 1;
		int ambientOcclusionMapIndex = 0;
		float tilingFactor = 1.0f;
    };

    DebugRenderSystem::DebugRenderSystem(Context& context, Shared<DescriptorAllocatorGrowable> descriptorAllocator, TextureRegistry& textureRegistry, VkRenderPass renderPass, DescriptorSetLayout& globalSetLayout)
		: m_context(context), m_descriptorAllocator(descriptorAllocator), m_textureRegistry(textureRegistry) {
        createPipelineLayout(globalSetLayout);
        createPipelines(renderPass);
    }

    DebugRenderSystem::~DebugRenderSystem() {
        vkDestroyPipelineLayout(m_context.getDevice(), m_pipelineLayout, nullptr);
    }

    void DebugRenderSystem::createPipelineLayout(DescriptorSetLayout& globalSetLayout) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(DebugPushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
            globalSetLayout.getDescriptorSetLayout(),
			m_textureRegistry.getDescriptorSetLayout()
        };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(m_context.getDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void DebugRenderSystem::createPipelines(VkRenderPass renderPass) {
        PXT_ASSERT(m_pipelineLayout != nullptr, "Cannot create pipeline before pipelineLayout");

        // Default Solid Pipeline
        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_pipelineLayout;

		const std::vector<std::pair<VkShaderStageFlagBits, std::string>>& shaderFilePaths = {
			{VK_SHADER_STAGE_VERTEX_BIT, SPV_SHADERS_PATH + "debug_shader.vert.spv"},
			{VK_SHADER_STAGE_FRAGMENT_BIT, SPV_SHADERS_PATH + "debug_shader.frag.spv"}
		};

        m_pipelineSolid = createUnique<Pipeline>(
            m_context,
            shaderFilePaths,
            pipelineConfig
        );

		// Wireframe Pipeline
		pipelineConfig.rasterizationInfo.polygonMode = VK_POLYGON_MODE_LINE;

		m_pipelineWireframe = createUnique<Pipeline>(
			m_context,
			shaderFilePaths,
			pipelineConfig
		);
    }

    void DebugRenderSystem::render(FrameInfo& frameInfo) {
		if (m_renderMode == Wireframe) {
			m_pipelineWireframe->bind(frameInfo.commandBuffer);
		}
		else {
			m_pipelineSolid->bind(frameInfo.commandBuffer);
		}

        std::array<VkDescriptorSet, 2> descriptorSets = {frameInfo.globalDescriptorSet, m_textureRegistry.getDescriptorSet()};

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipelineLayout,
            0,
            descriptorSets.size(),
            descriptorSets.data(),
            0,
            nullptr
        );

        auto view = frameInfo.scene.getEntitiesWith<TransformComponent, MeshComponent, MaterialComponent>();
        for (auto entity : view) {

            const auto&[transform, meshComponent, materialComponent] = view.get<TransformComponent, MeshComponent, MaterialComponent>(entity);

			auto material = materialComponent.material;
            auto vulkanMesh = std::static_pointer_cast<VulkanMesh>(meshComponent.mesh);

            DebugPushConstantData push{};
            push.modelMatrix = transform.mat4();
            push.normalMatrix = transform.normalMatrix();
			push.color = material->getAlbedoColor() * glm::vec4(materialComponent.tint, 1.0f);
			push.textureIndex = m_isAlbedoMapEnabled ? m_textureRegistry.getIndex(material->getAlbedoMap()->id) : -1;
			push.normalMapIndex = m_isNormalMapEnabled ? m_textureRegistry.getIndex(material->getNormalMap()->id) : -1;
			push.ambientOcclusionMapIndex = m_isAOMapEnabled ? m_textureRegistry.getIndex(material->getAmbientOcclusionMap()->id) : -1;
			push.tilingFactor = materialComponent.tilingFactor;

            push.enableWireframe = (uint32_t)(m_renderMode == Wireframe);
			push.enableNormals = (uint32_t)m_isNormalColorEnabled;
            

            vkCmdPushConstants(
                frameInfo.commandBuffer,
                m_pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(DebugPushConstantData),
                &push);
            
            vulkanMesh->bind(frameInfo.commandBuffer);
            vulkanMesh->draw(frameInfo.commandBuffer);

        }
    }

    void DebugRenderSystem::updateUi() {
		ImGui::Text("Render Mode:");
		ImGui::RadioButton("Wireframe", &m_renderMode, Wireframe);
        ImGui::RadioButton("Fill", &m_renderMode, Fill);
        ImGui::BeginDisabled(m_renderMode == Wireframe);
		ImGui::Checkbox("Show Normals as Color", &m_isNormalColorEnabled);
		ImGui::Checkbox("Show Albedo Map", &m_isAlbedoMapEnabled);
		ImGui::Checkbox("Show Normal Map", &m_isNormalMapEnabled);
		ImGui::Checkbox("Show Ambient Occlusion Map", &m_isAOMapEnabled);
		ImGui::EndDisabled();
    }
}