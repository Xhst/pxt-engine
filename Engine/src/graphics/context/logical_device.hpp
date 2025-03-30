#pragma once

#include <vector>

#include "graphics/window.hpp"
#include "graphics/context/instance.hpp"
#include "graphics/context/surface.hpp"
#include "graphics/context/physical_device.hpp"

namespace PXTEngine {

    class LogicalDevice {
       public:

        /**
         * @brief Constructor for the Device class.
         *
         * This constructor initializes the Vulkan device, including creating the instance,
         * setting up the debug messenger, creating the surface, picking a physical device,
         * creating the logical device, and creating the command pool.
         *
         * @param window The window to create the surface from.
         */
        LogicalDevice(Window& window, Instance& instance, Surface& surface, PhysicalDevice& physicalDevice);

        /**
         * @brief Destructor for the Device class.
         *
         * This destructor destroys the Vulkan device, including destroying the command pool,
         * logical device, debug messenger (if enabled), surface, and instance.
         */
        ~LogicalDevice();

        // Not copyable or movable
        LogicalDevice(const LogicalDevice&) = delete;
        void operator=(const LogicalDevice&) = delete;
        LogicalDevice(LogicalDevice&&) = delete;
        LogicalDevice &operator=(LogicalDevice&&) = delete;

        VkDevice getDevice() { return m_device; }

        VkQueue getGraphicsQueue() { return m_graphicsQueue; }
        VkQueue getPresentQueue() { return m_presentQueue; }

    private:

        /**
         * @brief Creates a logical device.
         *
         * This function creates a logical device, which is used to interact with the physical device.
         * It also creates the graphics and present queues.
         */
        void createLogicalDevice();

        Window& m_window;
        Instance& m_instance;
        Surface& m_surface;
        PhysicalDevice& m_physicalDevice;

        VkDevice m_device;
        
        VkQueue m_graphicsQueue;
        VkQueue m_presentQueue;
    };

}