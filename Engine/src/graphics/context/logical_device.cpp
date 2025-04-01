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

        // This structure holds the physical device features that are required for the logical device.
        VkPhysicalDeviceFeatures2 deviceFeatures2{};
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

        // Enable anisotropic filtering
        // A texture filtering technique that improves the quality of textures when viewed at oblique angles.
        deviceFeatures2.features.samplerAnisotropy = VK_TRUE;


        // Fetch the physical device features
        vkGetPhysicalDeviceFeatures2(m_physicalDevice.getDevice(), &deviceFeatures2);


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