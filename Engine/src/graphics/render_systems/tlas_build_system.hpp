#pragma once

#include "graphics/resources/blas_registry.hpp"
#include "graphics/resources/vk_buffer.hpp"
#include "graphics/frame_info.hpp"

namespace PXTEngine {
	class TLASBuildSystem {
	public:
		TLASBuildSystem(Context& context, BLASRegistry& blasRegistry);
		~TLASBuildSystem();

		// Delete the copy constructor and copy assignment operator
		TLASBuildSystem(const TLASBuildSystem&) = delete;
		TLASBuildSystem& operator=(const TLASBuildSystem&) = delete;

		void createTLAS(FrameInfo& frameInfo);
		void updateTLAS() {} // to implement later
	private:
		void destroyTLAS();
		VkTransformMatrixKHR glmToVkTransformMatrix(const glm::mat4& glmMatrix);

		Context& m_context;
		BLASRegistry& m_blasRegistry;

		VkAccelerationStructureKHR m_tlas = VK_NULL_HANDLE;
		Unique<VulkanBuffer> m_tlasBuffer;
		VkAccelerationStructureBuildSizesInfoKHR m_buildSizeInfo{};
		VkAccelerationStructureCreateInfoKHR m_createInfo{};
	};
}