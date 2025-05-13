#pragma once

#include "graphics/resources/blas_registry.hpp"
#include "graphics/resources/vk_buffer.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/descriptors/descriptors.hpp"

namespace PXTEngine {
	class TLASBuildSystem {
	public:
		TLASBuildSystem(Context& context, BLASRegistry& blasRegistry, Shared<DescriptorAllocatorGrowable> allocator);
		~TLASBuildSystem();

		// Delete the copy constructor and copy assignment operator
		TLASBuildSystem(const TLASBuildSystem&) = delete;
		TLASBuildSystem& operator=(const TLASBuildSystem&) = delete;

		void createTLAS(FrameInfo& frameInfo);
		void updateTLAS() {} // to implement later
		VkDescriptorSet getTLASDescriptorSet() const { return m_TLASDescriptorSet; }
		VkDescriptorSetLayout getTLASDescriptorSetLayout() const { return m_TLASDescriptorSetLayout->getDescriptorSetLayout(); }
	private:
		void destroyTLAS();
		VkTransformMatrixKHR glmToVkTransformMatrix(const glm::mat4& glmMatrix);
		void createDescriptorSetLayout();
		void createDescriptorSet();

		Context& m_context;
		BLASRegistry& m_blasRegistry;

		VkAccelerationStructureKHR m_tlas = VK_NULL_HANDLE;
		Unique<VulkanBuffer> m_tlasBuffer;
		VkAccelerationStructureBuildSizesInfoKHR m_buildSizeInfo{};
		VkAccelerationStructureCreateInfoKHR m_createInfo{};

		Shared<DescriptorAllocatorGrowable> m_descriptorAllocator;
		Shared<DescriptorSetLayout> m_TLASDescriptorSetLayout = nullptr;
		VkDescriptorSet m_TLASDescriptorSet = VK_NULL_HANDLE;
	};
}