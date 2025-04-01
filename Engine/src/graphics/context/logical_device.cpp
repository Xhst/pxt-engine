#include "graphics/context/logical_device.hpp"

#include <iostream>
#include <set>

namespace PXTEngine {

    LogicalDevice::LogicalDevice(Window& window, Instance& instance, Surface& surface, PhysicalDevice& physicalDevice)
		: m_window{ window }, m_instance{ instance }, m_surface(surface), m_physicalDevice(physicalDevice) {
        createLogicalDevice();
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

        // This structure is an extension structure that holds information about descriptor indexing features.
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


        // This structure holds the physical device features that are required for the logical device.
        VkPhysicalDeviceFeatures2 deviceFeatures2{};
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

        // Enable anisotropic filtering
        // A texture filtering technique that improves the quality of textures when viewed at oblique angles.
        deviceFeatures2.features.samplerAnisotropy = VK_TRUE;

        // Enable the descriptor indexing features
        deviceFeatures2.pNext = &indexingFeatures;


        // Fetch the physical device features
        vkGetPhysicalDeviceFeatures2(m_physicalDevice.getDevice(), &deviceFeatures2);

        // Check if the required features are supported
        if (!indexingFeatures.shaderSampledImageArrayNonUniformIndexing ||
            !indexingFeatures.descriptorBindingPartiallyBound ||
            !indexingFeatures.runtimeDescriptorArray) {
            throw std::runtime_error("Required descriptor indexing features are not supported!");
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
        createInfo.enabledExtensionCount = static_cast<uint32_t>(m_instance.deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = m_instance.deviceExtensions.data();

        
        if (vkCreateDevice(m_physicalDevice.getDevice(), &createInfo, nullptr,&m_device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(m_device, indices.graphicsFamily, 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device, indices.presentFamily, 0, &m_presentQueue);
    }
}