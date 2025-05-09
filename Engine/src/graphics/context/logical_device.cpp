#include "graphics/context/logical_device.hpp"

#include <iostream>
#include <set>

namespace PXTEngine {

    LogicalDevice::LogicalDevice(Window& window, Instance& instance, Surface& surface, PhysicalDevice& physicalDevice)
		: m_window{ window }, m_instance{ instance }, m_surface(surface), m_physicalDevice(physicalDevice) {
        createLogicalDevice();

        // Load ray tracing function pointers after the device is created -- global
        g_loadRayTracingFunctions(m_device);
    }

    LogicalDevice::~LogicalDevice() {
        vkDestroyDevice(m_device, nullptr);
    }

    void LogicalDevice::createLogicalDevice() {
        QueueFamilyIndices indices = m_physicalDevice.findQueueFamilies();

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // --- Feature Structures ---

        // Buffer Device Address Features (Required for RT)
        VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures{};
        bufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
        bufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;

		// Descriptor Indexing Features
        VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures{};
        indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;

        // This enables the ability to use non-uniform indexing for sampled image arrays within shaders.
        // Non-uniform indexing means that the index used to access an array can be dynamically calculated within 
        // the shader, rather than being a constant. 
        indexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;

        // This allows descriptor sets to have some bindings that are not bound to any resources.
        // This is useful for situations where you don't need to bind all resources in a descriptor set.
        indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;

        // This enables runtime-sized descriptor arrays, 
        // which means that the size of descriptor arrays can be determined dynamically at runtime.
        indexingFeatures.runtimeDescriptorArray = VK_TRUE;

        // Acceleration Structure Features
        VkPhysicalDeviceAccelerationStructureFeaturesKHR accelStructFeatures{};
        accelStructFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
        accelStructFeatures.accelerationStructure = VK_TRUE; // Enable this feature

        // Ray Tracing Pipeline Features
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtPipelineFeatures{};
        rtPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
        rtPipelineFeatures.rayTracingPipeline = VK_TRUE; // Enable this feature

        // --- Feature Chaining ---
        // Chain the features in this order 
        // BDA -> Descriptor Indexing -> Accel Struct -> RT Pipeline
        bufferDeviceAddressFeatures.pNext = &indexingFeatures;
        indexingFeatures.pNext = &accelStructFeatures;
        accelStructFeatures.pNext = &rtPipelineFeatures;
        rtPipelineFeatures.pNext = nullptr; // Make sure the last one points to nullptr

        // This structure holds the physical device features that are required for the logical device.
        VkPhysicalDeviceFeatures2 deviceFeatures2{};
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

        // Enable anisotropic filtering
        // A texture filtering technique that improves the quality of textures when viewed at oblique angles.
        deviceFeatures2.features.samplerAnisotropy = VK_TRUE;

		// Enable fill mode non solid for wireframe support
		deviceFeatures2.features.fillModeNonSolid = VK_TRUE;
  
        // Enable the descriptor indexing features
        deviceFeatures2.pNext = &bufferDeviceAddressFeatures;

        // Fetch the physical device features
        vkGetPhysicalDeviceFeatures2(m_physicalDevice.getDevice(), &deviceFeatures2);

        // Check if the required features are supported
        if (!indexingFeatures.shaderSampledImageArrayNonUniformIndexing ||
            !indexingFeatures.descriptorBindingPartiallyBound ||
            !indexingFeatures.runtimeDescriptorArray) {

            throw std::runtime_error("Required descriptor indexing features are not supported!");
        }

		// Check if the required features are supported
		if (!deviceFeatures2.features.samplerAnisotropy ||
            !deviceFeatures2.features.fillModeNonSolid) {
			throw std::runtime_error("Required features are not supported!");
		}

        if (!accelStructFeatures.accelerationStructure) {
            throw std::runtime_error("Required accelerationStructure feature is not supported!");
        }
        if (!rtPipelineFeatures.rayTracingPipeline) {
            throw std::runtime_error("Required rayTracingPipeline feature is not supported!");
        }

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        // These structures define the queues that the logical device will create. 
        // Queues are used for submitting work to the device, such as graphics commands or compute operations.
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        // This field is a pointer to an extension structure. 
        // It allows to chain additional information, enabling the use of Vulkan extensions. 
        // This is where you would place structures that enable newer Vulkan features.
        createInfo.pNext = &deviceFeatures2; 
        
        // pEnabledFeatures is the older, legacy way of specifying core Vulkan 1.0 features,
        // when using VkPhysicalDeviceFeatures2 set it to nullptr
        createInfo.pEnabledFeatures = nullptr;

        // Device extensions provide additional functionality beyond the core Vulkan specification.
        createInfo.enabledExtensionCount = static_cast<uint32_t>(m_physicalDevice.deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = m_physicalDevice.deviceExtensions.data();

        
        if (vkCreateDevice(m_physicalDevice.getDevice(), &createInfo, nullptr,&m_device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(m_device, indices.graphicsFamily, 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device, indices.presentFamily, 0, &m_presentQueue);
    }
}