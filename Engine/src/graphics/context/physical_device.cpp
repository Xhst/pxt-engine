#include "graphics/context/physical_device.hpp"

#include <vector>
#include <iostream>
#include <set>

namespace PXTEngine {

	/**
	 * @struct DeviceScore
	 * @brief Holds a Vulkan physical device and its suitability score.
	 *
	 * This structure is used to evaluate and rank physical devices based on their capabilities
	 * and suitability for the application's requirements. The score is used to select the best
	 * device for rendering operations.
	 */
    struct DeviceScore {
        static constexpr uint32_t DISCRETE_GPU_SCORING_POINTS = 150;
        static constexpr uint32_t INTEGRATED_GPU_SCORING_POINTS = 30;
        static constexpr uint32_t MB_REQUIRED_TO_SCORE_A_POINT = 100;

        VkPhysicalDevice device = VK_NULL_HANDLE;
        uint32_t score = 0;

        DeviceScore() = default;

        DeviceScore(const VkPhysicalDevice device, const uint32_t score)
    		: device(device), score(score) {}

        bool operator<(const DeviceScore& other) const {
            return score < other.score;
        }
    };

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

        std::cout << "Device count: " << deviceCount << '\n';

        std::vector<VkPhysicalDevice> devices(deviceCount);

        // Populates the devices vector with Vulkan physical device handles
        vkEnumeratePhysicalDevices(m_instance.getVkInstance(), &deviceCount, devices.data());

        DeviceScore bestDeviceScore;

        // Iterates through the available devices and checks if they are suitable for our needs.
        // Then calculate a score to select the best suitable GPU.
        for (const auto& device : devices) {
            VkPhysicalDeviceProperties currentDeviceProperties;
            vkGetPhysicalDeviceProperties(device, &currentDeviceProperties);

            if (!isDeviceSuitable(device)) {
				std::cout << currentDeviceProperties.deviceName << " is not suitable.\n";
				continue;
            }

			uint32_t currentScore = scoreDevice(device);

            std::cout << currentDeviceProperties.deviceName << ", Score: " << currentScore << '\n';

            if (currentScore > bestDeviceScore.score) {
                bestDeviceScore.device = device;
                bestDeviceScore.score = currentScore;
            }
        }
        std::cout << '\n';

        m_physicalDevice = bestDeviceScore.device;

        if (m_physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("Failed to find a suitable GPU!");
        }

        // This function retrieves detailed properties of the selected GPU, 
        // including its name, vendor, and supported Vulkan version.
        vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);

        std::cout << "Selected physical device: " << properties.deviceName << '\n';
    }

    uint32_t PhysicalDevice::scoreDevice(const VkPhysicalDevice device) {
        uint32_t score = 0;

        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        VkPhysicalDeviceMemoryProperties deviceMemoryProperties;

        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
        vkGetPhysicalDeviceMemoryProperties(device, &deviceMemoryProperties);

        // Add points based on the type of device (Discrete, Integrated)
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += DeviceScore::DISCRETE_GPU_SCORING_POINTS;
        } else if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            score += DeviceScore::INTEGRATED_GPU_SCORING_POINTS;
        }

        // Add points based on the amount of device-local memory
        uint64_t deviceLocalMemory = 0;
        for (uint32_t i = 0; i < deviceMemoryProperties.memoryHeapCount; ++i) {
            if (deviceMemoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                deviceLocalMemory += deviceMemoryProperties.memoryHeaps[i].size;
            }
        }

        uint32_t deviceLocalMemoryMb = static_cast<uint32_t>(deviceLocalMemory / 1048576);

        // Add points based on GPU memory size
        score += deviceLocalMemoryMb / DeviceScore::MB_REQUIRED_TO_SCORE_A_POINT;

        return score;
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

    bool PhysicalDevice::checkDeviceExtensionSupport(const VkPhysicalDevice device) {
        uint32_t extensionCount;

        // Gets the count of available device extensions.
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);

        // Populates the availableExtensions vector with the properties of each extension.
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        // Populate a set of available extension names for efficient lookup
        std::set<std::string> availableExtensionNames;
        for (const auto& extension : availableExtensions) {
            availableExtensionNames.insert(extension.extensionName);
        }

        // Flag to track if any essential (non-NV) required extension is missing
        bool allRequiredSupported = true;

        std::cout << "Required extensions not supported are:\n" << '\n';

        // Use the erase-remove idiom to filter the deviceExtensions vector
        deviceExtensions.erase(
            std::remove_if(deviceExtensions.begin(), deviceExtensions.end(),
                [&](const char* extNameCStr) {
                    // Convert const char* to std::string for easier comparison and checks
                    std::string extNameStr(extNameCStr);

                    // Check if this extension is available on the physical device
                    if (availableExtensionNames.count(extNameStr) == 0) {
                        // This extension is requested but NOT supported by the physical device

                        // Check if it's the NVIDIA-specific ray tracing validation extension
                        if (extNameStr.find("VK_NV") != std::string::npos) {
                            std::cout << extNameStr << " (OPTIONAL - Nvidia ext not supported, removed)\n";
                            return true; // Return true to mark this element for removal
                        }
                        else {
                            // This is a REQUIRED extension that is not supported
                            std::cout << extNameStr << " (REQUIRED - NOT SUPPORTED)\n";
                            allRequiredSupported = false; 
                            return false; // Keep this in the list so vkCreateDevice fails with its error
                        }
                    }
                    // If the extension IS available, we don't want to remove it
                    return false;
                }),
            deviceExtensions.end());

        return allRequiredSupported;
    }

    QueueFamilyIndices PhysicalDevice::findQueueFamiliesForDevice(const VkPhysicalDevice device) const {
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

    SwapChainSupportDetails PhysicalDevice::querySwapChainSupportForDevice(const VkPhysicalDevice device) const {
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
