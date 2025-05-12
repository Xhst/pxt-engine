#include "graphics/render_systems/raytracing_render_system.hpp"

namespace PXTEngine {
	RayTracingRenderSystem::RayTracingRenderSystem(Context& context, Shared<DescriptorAllocatorGrowable> descriptorAllocator, TextureRegistry& textureRegistry, BLASRegistry& blasRegistry, VkRenderPass renderPass, DescriptorSetLayout& globalSetLayout)
		: m_context(context),
		m_textureRegistry(textureRegistry),
		m_blasRegistry(blasRegistry),
		m_descriptorAllocator(descriptorAllocator) {}

	RayTracingRenderSystem::~RayTracingRenderSystem() {
		//vkDestroyPipelineLayout(m_context.getDevice(), m_pipelineLayout, nullptr);
	}

	void RayTracingRenderSystem::createShaderBindingTable() {}
	void RayTracingRenderSystem::createPipelineLayout(DescriptorSetLayout& setLayout) {}
	void RayTracingRenderSystem::createPipeline(VkRenderPass renderPass) {}

	void RayTracingRenderSystem::update(FrameInfo& frameInfo) {
		m_tlasBuildSystem.createTLAS(frameInfo);
	}

	void RayTracingRenderSystem::render(FrameInfo& frameInfo) {}
}