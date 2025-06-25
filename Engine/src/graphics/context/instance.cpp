#include "graphics/context/instance.hpp"

#include "core/logger.hpp"

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <unordered_set>
#include <sstream>

#ifndef ENABLE_RAYTRACING_EXT
#define ENABLE_RAYTRACING_EXT 1
#endif

namespace PXTEngine {

    /* ------------------------ Local callback functions ------------------------ */

    /**
     * @brief Debug callback function for Vulkan validation layers.
     *
     * This function is called by the Vulkan validation layers to report debug messages.
     * It prints the message to the standard error stream.
     *
     * @param messageSeverity The severity of the message.
     * @param messageType The type of the message.
     * @param pCallbackData Pointer to the message data.
     * @param pUserData Pointer to user data (unused).
     * @return VK_FALSE to indicate that the Vulkan call should not be aborted.
     */
    static VKAPI_ATTR VkBool32 VKAPI_CALL
    gpuDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,  void *pUserData) {

        std::stringstream ss;

        // Message type filtering (you can adjust these)
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
            ss << "\033[95m[GENERAL]\033[0m ";
        }
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
            ss << "\033[95m[VALIDATION]\033[0m ";
        }
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
            ss << "\033[95m[PERFORMANCE]\033[0m ";
        }

        ss << pCallbackData->pMessage << std::endl;

        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
			PXT_DEBUG(ss.str());
        } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
			PXT_INFO(ss.str());
        } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			PXT_WARN(ss.str());
        } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
			PXT_ERROR(ss.str());
        }

        return VK_FALSE;
    }

    /**
     * @brief Creates a Vulkan debug utils messenger.
     *
     * This function creates a debug utils messenger, which is used to receive debug messages
     * from the Vulkan validation layers.
     *
     * @param instance The Vulkan instance.
     * @param pCreateInfo Pointer to the create info structure.
     * @param pAllocator Pointer to the allocation callbacks.
     * @param pDebugMessenger Pointer to the debug messenger handle.
     * @return VK_SUCCESS if the messenger was created successfully, or a Vulkan error code.
     */
    VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
            const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger) {

        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

        if (func == nullptr) return VK_ERROR_EXTENSION_NOT_PRESENT;

        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }

    /**
     * @brief Destroys a Vulkan debug utils messenger.
     *
     * This function destroys a debug utils messenger.
     *
     * @param instance The Vulkan instance.
     * @param debugMessenger The debug messenger handle.
     * @param pAllocator Pointer to the allocation callbacks.
     */
    void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, 
            const VkAllocationCallbacks *pAllocator) {

        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    /* --------------------- End of local callback functions -------------------- */

    Instance::Instance(const std::string& appName) {
        createInstance(appName);
        setupDebugMessenger();
    }

    Instance::~Instance() {
        if (enableValidationLayers) {
            destroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
        }

        vkDestroyInstance(m_instance, nullptr);
    }

    std::vector<const char *> Instance::getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    void Instance::createInstance(const std::string& appName) {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = appName.c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "PXT Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;

        debugCreateInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | // Detailed diagnostic messages
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |    // Informational messages
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | // Potentially problematic usage
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;    // Invalid usage that may lead to crashes

        debugCreateInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |     // General events not tied to a specific Vulkan spec issue
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |  // Violations of the Vulkan spec or best practices
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;  // Potential non-optimal usage


        // Define the VkValidationFeaturesEXT for debug printf
        VkValidationFeatureEnableEXT enabledValidationFeatures[] = {
            VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT 
        };
        VkValidationFeaturesEXT validationFeatures{};
        validationFeatures.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
        validationFeatures.enabledValidationFeatureCount = 1;
        validationFeatures.pEnabledValidationFeatures = enabledValidationFeatures;

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);

            // chain
            createInfo.pNext = &debugCreateInfo;
            debugCreateInfo.pNext = &validationFeatures;
            validationFeatures.pNext = nullptr;

            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }

        hasGflwRequiredInstanceExtensions();
    }

    void Instance::setupDebugMessenger() {
        if (!enableValidationLayers) return;
        
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (createDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    void Instance::hasGflwRequiredInstanceExtensions() {
        uint32_t extensionCount = 0;

        // Get the number of available extensions
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> extensions(extensionCount);

        // Populate the extensions vector with the available extensions
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        std::cout << "available extensions:" << std::endl;
        std::unordered_set<std::string> available;

        for (const auto& extension : extensions) {
            std::cout << "\t" << extension.extensionName << std::endl;
            available.insert(extension.extensionName);
        }

        std::cout << "required extensions:" << std::endl;
        auto requiredExtensions = getRequiredExtensions();
        
        for (const auto& required : requiredExtensions) {
            std::cout << "\t" << required << std::endl;
            if (available.find(required) == available.end()) {
                throw std::runtime_error("Missing required glfw extension");
            }
        }
    }

    void Instance::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

        // defines the severity levels of messages that the debug messenger should listen to.
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        // specifies the types of debug messages to capture.
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT    |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        // specifies the callback function to be called when a debug message is generated.
        createInfo.pfnUserCallback = gpuDebugCallback;

        createInfo.pUserData = nullptr;  // Optional
    }

    bool Instance::checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount,
                                           availableLayers.data());

        for (const char *layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }
}