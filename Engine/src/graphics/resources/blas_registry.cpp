#include "graphics/resources/blas_registry.hpp"
#include "core/diagnostics.hpp"

namespace PXTEngine {
    BLASRegistry::BLASRegistry(Context& context) : m_context(context) {}

    BLASRegistry::~BLASRegistry() {
		// Clean up all BLAS resources
		for (auto& pair : m_blasRegistry) {
            Shared<BLAS>& blas = pair.second;
			if (blas->handle != VK_NULL_HANDLE) {
				vkDestroyAccelerationStructureKHR(m_context.getDevice(), blas->handle, nullptr);
			}
		}
	}
    
    Shared<BLAS> BLASRegistry::getOrCreateBLAS(Shared<Mesh>& mesh) {
        VulkanMesh* vkMesh_ptr = dynamic_cast<VulkanMesh*>(mesh.get());
		if (!vkMesh_ptr) {
			PXT_ERROR("Failed to cast Mesh to VulkanMesh");
			return nullptr;
		}

		VulkanMesh& vkMesh = *vkMesh_ptr;

		// Check if the BLAS already exists in the registry
		auto it = m_blasRegistry.find(vkMesh.id);
		if (it != m_blasRegistry.end()) {
			return it->second;
		}
        
        Shared<BLAS> newBlas = createBLAS(vkMesh);

		// Store the new BLAS in the registry
		m_blasRegistry[vkMesh.id] = newBlas;
		return newBlas;
    }

    VkAccelerationStructureGeometryKHR BLASRegistry::getAccelerationStructureGeometry(VulkanMesh& mesh) {
        VkDeviceAddress vertexBufferAddress = mesh.getVertexBufferDeviceAddress();

        bool meshHasIndexBuffer = mesh.getIndexCount() > 0;
        VkAccelerationStructureGeometryTrianglesDataKHR trianglesData{};
        trianglesData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
        trianglesData.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT; // Matches Mesh::Vertex::position
        trianglesData.vertexData.deviceAddress = vertexBufferAddress;
        trianglesData.vertexStride = sizeof(Mesh::Vertex);
        trianglesData.maxVertex = mesh.getVertexCount() - 1; // Max index in the vertex buffer
        trianglesData.indexType = meshHasIndexBuffer ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_NONE_KHR;
        trianglesData.indexData.deviceAddress = meshHasIndexBuffer ? mesh.getIndexBufferDeviceAddress() : 0;
        // transformData can be used for pre-transforming geometry within the BLAS, often identity or null here.
        // trianglesData.transformData.deviceAddress = 0;

        VkAccelerationStructureGeometryKHR geometry{};
        geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        geometry.geometry.triangles = trianglesData;
        // VK_GEOMETRY_OPAQUE_BIT_KHR is common for opaque geometry.
        // If you have non-opaque geometry, you might omit this or use any-hit shaders.
        geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

        return geometry;
    }

    Shared<BLAS> BLASRegistry::createBLAS(VulkanMesh& mesh) {
        Shared<BLAS> newBlas = createShared<BLAS>();
        VkDevice device = m_context.getDevice();

        // create Geometry Data BLAS
        newBlas->geometry = getAccelerationStructureGeometry(mesh);

        // for now just one geometry
        std::vector<VkAccelerationStructureGeometryKHR> geometries = { newBlas->geometry };

        // 2. Define Build Info (VkAccelerationStructureBuildGeometryInfoKHR)
        VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
        buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        buildInfo.geometryCount = static_cast<uint32_t>(geometries.size());
        buildInfo.pGeometries = geometries.data();

        // 3. Query Build Sizes (vkGetAccelerationStructureBuildSizesKHR)
        // to know how much space we need to store the BLAS AND how much space to create it (scratch buffer)
		bool hasIndices = mesh.getIndexCount() > 0;
        uint32_t numTriangles = hasIndices 
            ? (mesh.getIndexCount() / 3)
            : (mesh.getVertexCount() / 3);
        std::vector<uint32_t> maxPrimitiveCounts = { numTriangles };

        newBlas->buildSizes = {};
        newBlas->buildSizes.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
        vkGetAccelerationStructureBuildSizesKHR(
            device,
            VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
            &buildInfo,
            maxPrimitiveCounts.data(),
            &(newBlas->buildSizes)
        );
        
        // 4. Allocate BLAS Buffer and Scratch Buffer
        newBlas->buffer = createUnique<VulkanBuffer>(
            m_context, newBlas->buildSizes.accelerationStructureSize, 1,
            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        Unique<VulkanBuffer> scratchBuffer = createUnique<VulkanBuffer>(
            m_context, newBlas->buildSizes.buildScratchSize, 1,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        // BUILD BLAS HANDLE
        VkAccelerationStructureCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
        createInfo.buffer = newBlas->buffer->getBuffer(); // The VkBuffer handle of the storage buffer
        createInfo.offset = 0;                              // Offset within that buffer where the AS will be stored
        createInfo.size = newBlas->buildSizes.accelerationStructureSize; // Total size of the AS
        createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        // createInfo.deviceAddress = 0; // Only if VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR is set

        VkAccelerationStructureKHR blasHandle = VK_NULL_HANDLE;
        // Call the Vulkan function (via your context's loaded function pointer)
        VkResult result = vkCreateAccelerationStructureKHR(
            device,
            &createInfo,
            nullptr, // pAllocator
            &blasHandle
        );

        PXT_ASSERT(result == VK_SUCCESS, "Blas Handle creation error");
        
        newBlas->handle = blasHandle;

        // 5. Update Build Info with destination and scratch
        buildInfo.dstAccelerationStructure = newBlas->handle;
        buildInfo.scratchData.deviceAddress = scratchBuffer->getDeviceAddress();

        // 6. Build Command (vkCmdBuildAccelerationStructuresKHR)
        VkAccelerationStructureBuildRangeInfoKHR buildRangeInfo{};
        buildRangeInfo.primitiveCount = numTriangles;
        buildRangeInfo.primitiveOffset = 0;
        buildRangeInfo.firstVertex = 0;
        buildRangeInfo.transformOffset = 0; // Offset into transform data if used
        std::vector<const VkAccelerationStructureBuildRangeInfoKHR*> pBuildRangeInfos = { &buildRangeInfo };

        VkCommandBuffer commandBuffer = m_context.beginSingleTimeCommands();
        vkCmdBuildAccelerationStructuresKHR(
            commandBuffer,
            1, // buildInfoCount
            &buildInfo,
            pBuildRangeInfos.data()
        );

        // 7. Add barrier to ensure BLAS build completes before use / scratch buffer reuse
        VkMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR; // What the build command did
        barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;  // What subsequent operations will do (e.g., TLAS build reading the BLAS address)

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, // Stage where writing happened
            VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, // Stage where reading will happen (or other relevant stages like ray tracing shader stage)
            0,
            1, &barrier,
            0, nullptr,
            0, nullptr
        );

        m_context.endSingleTimeCommands(commandBuffer);

        //TODO: Maybe use a global scratch buffer as a class member instead of creating
        //      a new one every build, waiting for the build and then destroying it

        return newBlas;
    }
}