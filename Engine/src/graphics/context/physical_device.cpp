#include "graphics/context/physical_device.hpp"

#include <vector>
#include <iostream>
#include <set>

namespace PXTEngine {

    PhysicalDevice::PhysicalDevice(Instance& instance, Surface& surface) : m_instance(instance), m_surface(surface) {
        pickPhysicalDevice();
    }

    void PhysicalDevice::pickPhysicalDevice() {
        uint32_t deviceCount = 0;

        // Retrieves the number of physical devices (GPUs) that support Vulkan.
        vkEnumeratePhysicalDevices(m_instance.getVkInstance(), &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");
        }

        std::cout << "Device count: " << deviceCount << std::endl;

        std::vector<VkPhysicalDevice> devices(deviceCount);

        // Populates the devices vector with Vulkan physical device handles
        vkEnumeratePhysicalDevices(m_instance.getVkInstance(), &deviceCount, devices.data());

        // Iterates through the available devices and checks if they are suitable for our needs.
        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                m_physicalDevice = device;
                break;
            }
        }

        if (m_physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("Failed to find a suitable GPU!");
        }

        // This function retrieves detailed properties of the selected GPU, 
        // including its name, vendor, and supported Vulkan version.
        vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);

        std::cout << "Physical device: " << properties.deviceName << std::endl;
    }

    bool PhysicalDevice::isDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices indices = findQueueFamiliesForDevice(device);

        bool extensionsSupported = checkDeviceExtensionSupport(device);
        bool swapChainAdequate = false;

        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupportForDevice(device);

            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }

    bool PhysicalDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;

        // Gets the count of available device extensions.
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);

        // Populates the availableExtensions vector with the properties of each extension.
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(m_instance.deviceExtensions.begin(), m_instance.deviceExtensions.end());

        // Removes any extension from requiredExtensions that is found in the available extensions list.
        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    QueueFamilyIndices PhysicalDevice::findQueueFamiliesForDevice(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;

        // Gets the count of available queue families for the device.
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);

        // Populates the queueFamilies vector with the properties of each queue family.
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
                indices.graphicsFamilyHasValue = true;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface.getSurface(), &presentSupport);

            if (queueFamily.queueCount > 0 && presentSupport) {
                indices.presentFamily = i;
                indices.presentFamilyHasValue = true;
            }

            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }

    SwapChainSupportDetails PhysicalDevice::querySwapChainSupportForDevice(VkPhysicalDevice device) {
        SwapChainSupportDetails details;

        // Queries the surface capabilities, formats, and present modes for the device.
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface.getSurface(), &details.capabilities);

        uint32_t formatCount;

        // Gets the count of available surface formats.
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface.getSurface(), &formatCount, nullptr);

        // If the count is not zero, resize the formats vector and populate it with the available formats.
        if (formatCount != 0) {
            details.formats.resize(formatCount);

            vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface.getSurface(), &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;

        // Gets the count of available present modes.
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface.getSurface(), &presentModeCount, nullptr);

        // If the count is not zero, resize the presentModes vector and populate it with the available present modes.
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface.getSurface(), &presentModeCount, details.presentModes.data());
        }

        return details;
    }

}
