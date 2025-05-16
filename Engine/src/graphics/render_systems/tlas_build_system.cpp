#include "graphics/render_systems/tlas_build_system.hpp"

#include "scene/ecs/component.hpp"

namespace PXTEngine {
	TLASBuildSystem::TLASBuildSystem(Context& context, BLASRegistry& blasRegistry, Shared<DescriptorAllocatorGrowable> allocator)
		: m_context(context), m_blasRegistry(blasRegistry), m_descriptorAllocator(allocator) {
		createDescriptorSet();
	}

	TLASBuildSystem::~TLASBuildSystem() {
		destroyTLAS();
	}

	void TLASBuildSystem::createTLAS(FrameInfo& frameInfo) {
		VkAccelerationStructureKHR newTlas = VK_NULL_HANDLE;

		//  Create a acceleration structure instance vector 
		std::vector<VkAccelerationStructureInstanceKHR> instances;

		//  Get all BLAS and components from entities that have transform & mesh components 
		auto view = frameInfo.scene.getEntitiesWith<TransformComponent, MeshComponent, MaterialComponent>();
		//TODO: replace with actual instanceCustomIndex based on material?
		uint32_t instanceIndex = 0;

		for (auto entity : view) {
			const auto& [transformComponent, meshComponent, materialComponent] = view.get<TransformComponent, MeshComponent, MaterialComponent>(entity);
			
			auto material = materialComponent.material;
			auto mesh = meshComponent.mesh;

			Shared<BLAS> blas = m_blasRegistry.getOrCreateBLAS(mesh);

			VkAccelerationStructureDeviceAddressInfoKHR addressInfo{};
			addressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
			addressInfo.accelerationStructure = blas->handle;
			VkDeviceAddress blasAddress = blas->buffer->getDeviceAddress();

			// convert glm::mat4 to VkTransformMatrixKHR
			VkTransformMatrixKHR transformMatrix = glmToVkTransformMatrix(transformComponent.mat4());

			// Define the instance
			VkAccelerationStructureInstanceKHR instance{};
			instance.transform = transformMatrix;
			instance.instanceCustomIndex = instanceIndex; // Example: Use loop index
			instance.mask = 0xFF; // Visible to all rays initially
			instance.instanceShaderBindingTableRecordOffset = 0; // Example: Use base offset
			instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR; // Example flags
			instance.accelerationStructureReference = blasAddress;

			instances.push_back(instance);
			instanceIndex++;
		}

		// Upload Instance Data 
		uint32_t instanceCount = static_cast<uint32_t>(instances.size());
		VkDeviceSize instanceDataSize = sizeof(VkAccelerationStructureInstanceKHR) * instanceCount;
		Unique<VulkanBuffer> instanceBuffer;
		Unique<VulkanBuffer> stagingBuffer;

		// Create staging buffer
		stagingBuffer = createUnique<VulkanBuffer>(
			m_context,
			sizeof(VkAccelerationStructureInstanceKHR),
			instanceCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		stagingBuffer->map();
		stagingBuffer->writeToBuffer((void*)instances.data());

		// Create instance buffer on GPU
		instanceBuffer = createUnique<VulkanBuffer>(
			m_context,
			sizeof(VkAccelerationStructureInstanceKHR),
			instanceCount,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT |
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
			VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		// Copy instance data to GPU
		m_context.copyBuffer(stagingBuffer->getBuffer(), instanceBuffer->getBuffer(), instanceDataSize);

		// Query Build Sizes
		VkDeviceAddress instanceBufferAddr = instanceBuffer->getDeviceAddress();
		
		VkAccelerationStructureGeometryInstancesDataKHR instancesGeometryData{};
		instancesGeometryData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		instancesGeometryData.arrayOfPointers = VK_FALSE; // Instance data is tightly packed
		instancesGeometryData.data.deviceAddress = instanceBufferAddr;

		VkAccelerationStructureGeometryKHR TLASGeometry{};
		TLASGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		TLASGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		TLASGeometry.geometry.instances = instancesGeometryData;
		// Flags like VK_GEOMETRY_OPAQUE_BIT_KHR could be set here if needed

		VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
		buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR; // Or other flags
		buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		buildInfo.geometryCount = 1;
		buildInfo.pGeometries = &TLASGeometry;

		uint32_t numInstances = static_cast<uint32_t>(instances.size());
		VkAccelerationStructureBuildSizesInfoKHR m_buildSizeInfo{};
		m_buildSizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		vkGetAccelerationStructureBuildSizesKHR(
			m_context.getDevice(),
			VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			&buildInfo,
			&numInstances, // Pointer to the number of primitives (instances)
			&m_buildSizeInfo);


		// 4. Allocate BLAS Buffer and Scratch Buffer
		m_tlasBuffer = createUnique<VulkanBuffer>(
			m_context, 
			m_buildSizeInfo.accelerationStructureSize,
			1,
			VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		Unique<VulkanBuffer> scratchBuffer = createUnique<VulkanBuffer>(
			m_context,
			m_buildSizeInfo.buildScratchSize,
			1,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		VkDeviceAddress scratchBufferAddr = scratchBuffer->getDeviceAddress();

		//  5. Create TLAS Object 
		VkAccelerationStructureCreateInfoKHR m_createInfo{};
		m_createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		m_createInfo.buffer = m_tlasBuffer->getBuffer();
		m_createInfo.offset = 0;
		m_createInfo.size = m_buildSizeInfo.accelerationStructureSize;
		m_createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		// createInfo.deviceAddress = // Optional: if using buffer device address capture/replay

		if (vkCreateAccelerationStructureKHR(m_context.getDevice(), &m_createInfo, nullptr, &newTlas) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create top-level acceleration structure!");
		}

		//  6. Build TLAS Command 
		VkCommandBuffer commandBuffer = m_context.beginSingleTimeCommands();

		// Update build info with destination and scratch
		buildInfo.dstAccelerationStructure = newTlas;
		buildInfo.scratchData.deviceAddress = scratchBufferAddr;

		// Define the build range (how many instances to build)
		VkAccelerationStructureBuildRangeInfoKHR buildRangeInfos{};
		buildRangeInfos.primitiveCount = numInstances;
		buildRangeInfos.primitiveOffset = 0;
		buildRangeInfos.firstVertex = 0;
		buildRangeInfos.transformOffset = 0;
		const VkAccelerationStructureBuildRangeInfoKHR* pBuildRangeInfos = &buildRangeInfos;

		// Record the build command
		vkCmdBuildAccelerationStructuresKHR(
			commandBuffer,
			1,               // buildInfoCount
			&buildInfo,      // pBuildInfo
			&pBuildRangeInfos // ppBuildRangeInfos
		);

		// Add a barrier to ensure BLAS builds are complete and instance buffer write is complete
		VkMemoryBarrier memoryBarrier = {};
		memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR; // Instance buffer copy + BLAS builds
		memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR; // TLAS build reads instance buffer & BLASes

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, // Source stages
			VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,                                 // Destination stage
			0, // Dependency flags
			1, &memoryBarrier, // Memory barriers
			0, nullptr, // Buffer memory barriers
			0, nullptr  // Image memory barriers
		);

		m_context.endSingleTimeCommands(commandBuffer); // Submits and waits

		//  Cleanup 
		// Destroy the scratch buffer (it's only needed during build) and other buffers like
		// instance buffer or staging buffer. we can potentially keep the instance buffer for reuse.
		// Buffers will be deleted after end of this function cause they are Unique.

		// Update descriptor set for TLAS
		updateDescriptorSet(newTlas);

		// Then destroy old one and assign the new one
		destroyTLAS();

		// If not done in this order it will give a validation error
		// because the TLAS is still in use by the descriptor when we destroy it.
		// TODO: add a list of SWAPCHAIN::MAX_FRAMES_IN_FLIGHT tlases with their descriptor sets

		m_tlas = newTlas;
	}

	VkTransformMatrixKHR TLASBuildSystem::glmToVkTransformMatrix(const glm::mat4& glmMatrix) {
		VkTransformMatrixKHR vkMatrix;

		// GLM matrices are column-major, VkTransformMatrixKHR is row-major.
		// We need to transpose the 3x4 part and assign.
		// The last row of a 4x4 homogeneous matrix (0,0,0,1) is omitted.

		// vkMatrix.matrix[row][column]
		// glmMatrix[column][row] (due to column-major order)

		for (int row = 0; row < 3; ++row) {
			for (int col = 0; col < 4; ++col) {
				vkMatrix.matrix[row][col] = glmMatrix[col][row];
			}
		}

		return vkMatrix;
	}

	void TLASBuildSystem::createDescriptorSet() {
		// TLAS DESCRIPTOR SET LAYOUT
		// needed for raytracing pipeline layout
		m_tlasDescriptorSetLayout = DescriptorSetLayout::Builder(m_context)
			.addBinding(0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, VK_SHADER_STAGE_RAYGEN_BIT_KHR)
			.build();

		m_descriptorAllocator->allocate(m_tlasDescriptorSetLayout->getDescriptorSetLayout(), m_tlasDescriptorSet);
	}

	void TLASBuildSystem::updateDescriptorSet(VkAccelerationStructureKHR& newTlas) {
		VkWriteDescriptorSetAccelerationStructureKHR tlasInfo{};
		tlasInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		tlasInfo.accelerationStructureCount = 1;
		tlasInfo.pAccelerationStructures = &newTlas;

		DescriptorWriter(m_context, *m_tlasDescriptorSetLayout)
			.writeTLAS(0, tlasInfo)
			.updateSet(m_tlasDescriptorSet);
	}

	void TLASBuildSystem::destroyTLAS() {
		if (m_tlas != VK_NULL_HANDLE) {
			vkDestroyAccelerationStructureKHR(m_context.getDevice(), m_tlas, nullptr);
			m_tlas = VK_NULL_HANDLE;
		}
	}
}