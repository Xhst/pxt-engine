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

        VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures{};
        indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
        indexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
        indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
        indexingFeatures.runtimeDescriptorArray = VK_TRUE;

        VkPhysicalDeviceFeatures2 deviceFeatures2{};
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures2.pNext = &indexingFeatures;

        vkGetPhysicalDeviceFeatures2(m_physicalDevice.getDevice(), &deviceFeatures2);

        // Check if the required features are supported
        if (!indexingFeatures.shaderSampledImageArrayNonUniformIndexing ||
            !indexingFeatures.descriptorBindingPartiallyBound ||
            !indexingFeatures.runtimeDescriptorArray) {
            throw std::runtime_error("Required descriptor indexing features are not supported!");
        }

        VkPhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.samplerAnisotropy = VK_TRUE; 


        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount =
            static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pNext = &deviceFeatures2; // Attach extended features

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount =
            static_cast<uint32_t>(m_instance.deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = m_instance.deviceExtensions.data();

        // might not really be necessary anymore because device specific
        // validation layers have been deprecated
        if (m_instance.enableValidationLayers) {
            createInfo.enabledLayerCount =
                static_cast<uint32_t>(m_instance.validationLayers.size());
            createInfo.ppEnabledLayerNames = m_instance.validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(m_physicalDevice.getDevice(), &createInfo, nullptr,
                           &m_device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(m_device, indices.graphicsFamily, 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device, indices.presentFamily, 0, &m_presentQueue);
    }
}