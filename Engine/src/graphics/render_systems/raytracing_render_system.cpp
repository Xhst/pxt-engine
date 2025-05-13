#include "graphics/render_systems/raytracing_render_system.hpp"

#include "core/constants.hpp"

namespace PXTEngine {
	RayTracingRenderSystem::RayTracingRenderSystem(
		Context& context, Shared<DescriptorAllocatorGrowable> descriptorAllocator,
		TextureRegistry& textureRegistry, BLASRegistry& blasRegistry,
		DescriptorSetLayout& globalSetLayout)
		: m_context(context),
		m_textureRegistry(textureRegistry),
		m_blasRegistry(blasRegistry),
		m_descriptorAllocator(descriptorAllocator) {
		createDescriptorSets();
		defineShaderGroups();
		createPipelineLayout(globalSetLayout);
		createPipeline();
		createShaderBindingTable();
	}

	RayTracingRenderSystem::~RayTracingRenderSystem() {
		vkDestroyPipelineLayout(m_context.getDevice(), m_pipelineLayout, nullptr);
	}


	void RayTracingRenderSystem::createDescriptorSets() {
		
	}

	void RayTracingRenderSystem::defineShaderGroups() {
		m_shaderGroups = {
			// General Shader Group
			{
				VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
				{
					// Shader stages + filepaths
					{VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_SHADERS_PATH + "primary.rgen.spv"},
					{VK_SHADER_STAGE_MISS_BIT_KHR, SPV_SHADERS_PATH + "primary.rmiss.spv"}
				}
			},
			// Closest Hit Group (Triangle Hit Group)
			{
				VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR,
				{
					// Shader stages + filepaths
					{VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_SHADERS_PATH + "primary.rchit.spv"}
				}
			}
		};
	}

	void RayTracingRenderSystem::createPipelineLayout(DescriptorSetLayout& setLayout) {
		// do we need push constants? (yes / no)
		/*
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(MaterialPushConstantData);
		*/

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
			setLayout.getDescriptorSetLayout(),
			m_tlasBuildSystem.getTLASDescriptorSetLayout(),
			m_textureRegistry.getDescriptorSetLayout()
		};

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 0; // 0 for now
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // &pushConstantRange;

		if (vkCreatePipelineLayout(m_context.getDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create raytracingRenderSystem pipeline layout!");
		}
	}

	void RayTracingRenderSystem::createPipeline() {
		RayTracingPipelineConfigInfo pipelineConfig{};
		pipelineConfig.shaderGroups = m_shaderGroups;
		pipelineConfig.pipelineLayout = m_pipelineLayout;
		pipelineConfig.maxPipelineRayRecursionDepth = 1; // for now
		m_pipeline = createUnique<Pipeline>(
			m_context,
			pipelineConfig
		);
	}

	void RayTracingRenderSystem::createShaderBindingTable() {}
	
	void RayTracingRenderSystem::update(FrameInfo& frameInfo) {
		m_tlasBuildSystem.createTLAS(frameInfo);
	}

	void RayTracingRenderSystem::render(FrameInfo& frameInfo) {
		m_pipeline->bind(frameInfo.commandBuffer);

		std::array<VkDescriptorSet, 3> descriptorSets = { frameInfo.globalDescriptorSet, m_tlasBuildSystem.getTLASDescriptorSet(), m_textureRegistry.getDescriptorSet()};
	}
}