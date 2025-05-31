#pragma once

#include <vulkan/vulkan.h>
// Include necessary extension headers if not already pulled in by vulkan.h
// For Vulkan 1.3, many are promoted, but explicit includes might be needed depending on SDK/headers
// #include <vulkan/vulkan_beta.h> // Might be needed for some definitions if not using latest headers

// Declare extern function pointers for ray tracing functions

// Acceleration Structure functions
extern PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR_;
extern PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR_;
extern PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR_;
extern PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR_;

// add if needed later (^_^)
/*
extern PFN_vkCmdCopyAccelerationStructureKHR vkCmdCopyAccelerationStructureKHR_;
extern PFN_vkCopyAccelerationStructureToMemoryKHR vkCopyAccelerationStructureToMemoryKHR_;
extern PFN_vkCopyMemoryToAccelerationStructureKHR vkCopyMemoryToAccelerationStructureKHR_;
extern PFN_vkWriteAccelerationStructuresPropertiesKHR vkWriteAccelerationStructuresPropertiesKHR_;
extern PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR_;
*/

// Ray Tracing Pipeline functions
extern PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR_;
extern PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR_;
extern PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR_;

// Add more ray tracing or related extension functions as needed (e.g., SBT functions)
// extern PFN_vkGetRayTracingShaderGroupStackSizeKHR vkGetRayTracingShaderGroupStackSizeKHR_;
// extern PFN_vkCmdSetRayTracingPipelineStackSizeKHR vkCmdSetRayTracingPipelineStackSizeKHR_;


// Use #define to map the standard Vulkan function names to our global pointers
// This allows you to call them like regular Vulkan functions

// Acceleration Structure functions
#define vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR_
#define vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR_
#define vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR_
#define vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR_

// add if needed later (^_^)
/*
#define vkCmdCopyAccelerationStructureKHR vkCmdCopyAccelerationStructureKHR_
#define vkCopyAccelerationStructureToMemoryKHR vkCopyAccelerationStructureToMemoryKHR_
#define vkCopyMemoryToAccelerationStructureKHR vkCopyMemoryToAccelerationStructureKHR_
#define vkWriteAccelerationStructuresPropertiesKHR vkWriteAccelerationStructuresPropertiesKHR_
#define vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR_
*/

// Ray Tracing Pipeline functions
#define vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR_
#define vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR_
#define vkCmdTraceRaysKHR vkCmdTraceRaysKHR_

// Add #defines for other functions declared above
// #define vkGetRayTracingShaderGroupStackSizeKHR vkGetRayTracingShaderGroupStackSizeKHR_
// #define vkCmdSetRayTracingPipelineStackSizeKHR vkCmdSetRayTracingPipelineStackSizeKHR_


// Function to load all required ray tracing and BDA function pointers
void g_loadRayTracingFunctions(VkDevice device);