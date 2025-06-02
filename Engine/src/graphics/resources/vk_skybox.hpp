#pragma once

#include "graphics/resources/cube_map.hpp"
#include "graphics/descriptors/descriptors.hpp"
#include "scene/skybox.hpp"

namespace PXTEngine {

	class VulkanSkybox : public Skybox {
	public:
		static Unique<VulkanSkybox> create(const std::array<std::string, 6>& paths);

		VulkanSkybox(Context& context, const std::array<std::string, 6>& paths);

		~VulkanSkybox() override = default;

		void createDescriptorSet(Shared<DescriptorAllocatorGrowable> descriptorAllocator);

		VkDescriptorImageInfo getDescriptorImageInfo() const;
		VkDescriptorSet getDescriptorSet() const { return m_skyboxDescriptorSet; }
		VkDescriptorSetLayout getDescriptorSetLayout() const { return m_skyboxDescriptorSetLayout->getDescriptorSetLayout(); }

	private:
		void loadTextures(const std::array<std::string, 6>& paths);

		Context& m_context;
		
		uint32_t m_size = 0;
		Unique<CubeMap> m_cubeMap;

		VkDescriptorSet m_skyboxDescriptorSet;
		Unique<DescriptorSetLayout> m_skyboxDescriptorSetLayout;
	};
}