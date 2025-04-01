#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

namespace PXTEngine {

    /**
     * @class Instance
     * 
     * @brief Manages the Vulkan instance and its associated resources.
     * 
     * This class is responsible for creating and managing the Vulkan instance, setting up validation layers,
     * and handling debug messages. It also provides methods to retrieve the Vulkan instance and required extensions.
     */
    class Instance {
    public:
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
        Instance(const std::string& appName);
        ~Instance();

        Instance(const Instance&) = delete;
        Instance& operator=(const Instance&) = delete;

        VkInstance getVkInstance() { return m_instance; }

        /**
         * @brief Gets the required extensions for the Vulkan instance.
         *
         * This function gets the required extensions for the Vulkan instance, including the GLFW extensions
         * and the debug utils extension if validation layers are enabled.
         *
         * @return A vector of required extensions.
         */
        std::vector<const char*> getRequiredExtensions();

        const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    private:
        /**
         * @brief Creates a Vulkan instance.
         *
         * This function creates a Vulkan instance, which is the entry point for all Vulkan commands.
         * It also sets up the validation layers if they are enabled.
         * 
         * @param appName The name of the application.
         */
        void createInstance(const std::string& appName);

        /**
         * @brief Checks if the required GLFW extensions are supported.
         *
         * This function checks if all the required GLFW extensions are supported by the Vulkan instance.
         */
        void hasGflwRequiredInstanceExtensions();

        /**
         * @brief Populates the debug messenger create info structure.
         *
         * This function populates the debug messenger create info structure with the required parameters.
         *
         * @param createInfo The debug messenger create info structure.
         */
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

        /**
         * @brief Checks if the validation layers are supported.
         *
         * This function checks if all the required validation layers are supported by the Vulkan instance.
         *
         * @return true if all layers are supported, false otherwise.
         */
        bool checkValidationLayerSupport();

        /**
         * @brief Sets up the debug messenger.
         *
         * This function creates a debug messenger if validation layers are enabled.
         */
        void setupDebugMessenger();


        VkDebugUtilsMessengerEXT m_debugMessenger;

        VkInstance m_instance;
    };
}