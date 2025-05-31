#pragma once

#include <unordered_map>

#include "core/memory.hpp"
#include "graphics/context/context.hpp"
#include "graphics/resources/vk_buffer.hpp"
#include "graphics/resources/vk_mesh.hpp"

namespace PXTEngine {
	struct BLAS {
		VkAccelerationStructureKHR handle = VK_NULL_HANDLE;
		VkAccelerationStructureBuildSizesInfoKHR buildSizes;
		VkAccelerationStructureGeometryKHR geometry;
		Unique<VulkanBuffer> buffer;

		bool operator==(const BLAS& other) const {
			return handle == other.handle;
		}
	};

	class BLASRegistry {
	public:
		BLASRegistry(Context& context);
		~BLASRegistry();
		BLASRegistry(const BLASRegistry&) = delete;
		BLASRegistry& operator=(const BLASRegistry&) = delete;

		Shared<BLAS> getOrCreateBLAS(Shared<Mesh>& mesh);
	private:
		VkAccelerationStructureGeometryKHR getAccelerationStructureGeometry(VulkanMesh& mesh);
		Shared<BLAS> createBLAS(VulkanMesh& mesh);

		Context& m_context;
		std::unordered_map<UUID, Shared<BLAS>> m_blasRegistry;
	};
}