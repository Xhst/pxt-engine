#include "graphics/context/ray_tracing_vk_ext_func.hpp"

#include <stdexcept>
#include <iostream>

// Define the global function pointers, initialized to nullptr

// Acceleration Structure functions
PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR_ = nullptr;
PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR_ = nullptr;
PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR_ = nullptr;
PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR_ = nullptr;

// add if needed later (^_^)
/*
PFN_vkCmdCopyAccelerationStructureKHR vkCmdCopyAccelerationStructureKHR_ = nullptr;
PFN_vkCopyAccelerationStructureToMemoryKHR vkCopyAccelerationStructureToMemoryKHR_ = nullptr;
PFN_vkCopyMemoryToAccelerationStructureKHR vkCopyMemoryToAccelerationStructureKHR_ = nullptr;
PFN_vkWriteAccelerationStructuresPropertiesKHR vkWriteAccelerationStructuresPropertiesKHR_ = nullptr;
PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR_ = nullptr;
*/

// Ray Tracing Pipeline functions
PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR_ = nullptr;
PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR_ = nullptr;
PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR_ = nullptr;

// Define other function pointers as declared in the header


// Function to load all required ray tracing and BDA function pointers
void g_loadRayTracingFunctions(VkDevice device) {
    // Use vkGetDeviceProcAddr to load device-level extension functions

    // Acceleration Structure functions
    vkCreateAccelerationStructureKHR_ = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(
        vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR"));
    if (!vkCreateAccelerationStructureKHR_) throw std::runtime_error("Failed to load vkCreateAccelerationStructureKHR");

    vkDestroyAccelerationStructureKHR_ = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(
        vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR"));
    if (!vkDestroyAccelerationStructureKHR_) throw std::runtime_error("Failed to load vkDestroyAccelerationStructureKHR");

    vkCmdBuildAccelerationStructuresKHR_ = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(
        vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresKHR"));
    if (!vkCmdBuildAccelerationStructuresKHR_) throw std::runtime_error("Failed to load vkCmdBuildAccelerationStructuresKHR");

	vkGetAccelerationStructureBuildSizesKHR_ = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(
		vkGetDeviceProcAddr(device, "vkGetAccelerationStructureBuildSizesKHR"));
	if (!vkGetAccelerationStructureBuildSizesKHR_) throw std::runtime_error("Failed to load vkGetAccelerationStructureBuildSizesKHR");

	// add if needed later (^_^)
    /*
    vkCmdCopyAccelerationStructureKHR_ = reinterpret_cast<PFN_vkCmdCopyAccelerationStructureKHR>(
        vkGetDeviceProcAddr(device, "vkCmdCopyAccelerationStructureKHR"));
    if (!vkCmdCopyAccelerationStructureKHR_) throw std::runtime_error("Failed to load vkCmdCopyAccelerationStructureKHR");

    vkCopyAccelerationStructureToMemoryKHR_ = reinterpret_cast<PFN_vkCopyAccelerationStructureToMemoryKHR>(
        vkGetDeviceProcAddr(device, "vkCopyAccelerationStructureToMemoryKHR"));
    if (!vkCopyAccelerationStructureToMemoryKHR_) throw std::runtime_error("Failed to load vkCopyAccelerationStructureToMemoryKHR");

    vkCopyMemoryToAccelerationStructureKHR_ = reinterpret_cast<PFN_vkCopyMemoryToAccelerationStructureKHR>(
        vkGetDeviceProcAddr(device, "vkCopyMemoryToAccelerationStructureKHR"));
    if (!vkCopyMemoryToAccelerationStructureKHR_) throw std::runtime_error("Failed to load vkCopyMemoryToAccelerationStructureKHR");

    vkWriteAccelerationStructuresPropertiesKHR_ = reinterpret_cast<PFN_vkWriteAccelerationStructuresPropertiesKHR>(
        vkGetDeviceProcAddr(device, "vkWriteAccelerationStructuresPropertiesKHR"));
    if (!vkWriteAccelerationStructuresPropertiesKHR_) throw std::runtime_error("Failed to load vkWriteAccelerationStructuresPropertiesKHR");

    vkGetAccelerationStructureDeviceAddressKHR_ = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(
        vkGetDeviceProcAddr(device, "vkGetAccelerationStructureDeviceAddressKHR"));
    if (!vkGetAccelerationStructureDeviceAddressKHR_) throw std::runtime_error("Failed to load vkGetAccelerationStructureDeviceAddressKHR");
    */

    // Ray Tracing Pipeline functions
    vkCreateRayTracingPipelinesKHR_ = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(
        vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesKHR"));
    if (!vkCreateRayTracingPipelinesKHR_) throw std::runtime_error("Failed to load vkCreateRayTracingPipelinesKHR");

    vkGetRayTracingShaderGroupHandlesKHR_ = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(
        vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesKHR"));
    if (!vkGetRayTracingShaderGroupHandlesKHR_) throw std::runtime_error("Failed to load vkGetRayTracingShaderGroupHandlesKHR");

    vkCmdTraceRaysKHR_ = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(
        vkGetDeviceProcAddr(device, "vkCmdTraceRaysKHR"));
    if (!vkCmdTraceRaysKHR_) throw std::runtime_error("Failed to load vkCmdTraceRaysKHR");

    // Load other functions as needed
    // vkGetRayTracingShaderGroupStackSizeKHR_ = reinterpret_cast<PFN_vkGetRayTracingShaderGroupStackSizeKHR>(
    //     vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupStackSizeKHR"));
    // if (!vkGetRayTracingShaderGroupStackSizeKHR_) throw std::runtime_error("Failed to load vkGetRayTracingShaderGroupStackSizeKHR");

    // vkCmdSetRayTracingPipelineStackSizeKHR_ = reinterpret_cast<PFN_vkCmdSetRayTracingPipelineStackSizeKHR>(
    //     vkGetDeviceProcAddr(device, "vkCmdSetRayTracingPipelineStackSizeKHR"));
    // if (!vkCmdSetRayTracingPipelineStackSizeKHR_) throw std::runtime_error("Failed to load vkCmdSetRayTracingPipelineStackSizeKHR");

    std::cout << "Successfully loaded Vulkan Ray Tracing and BDA function pointers.\n";
}

