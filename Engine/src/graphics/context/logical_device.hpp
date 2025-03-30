#pragma once

#include <vector>

#include "graphics/window.hpp"
#include "graphics/context/instance.hpp"
#include "graphics/context/surface.hpp"
#include "graphics/context/physical_device.hpp"

namespace PXTEngine {

    /**
     * @class LogicalDevice
     * 
     * @brief Manages the Vulkan logical device and its associated resources.
     * 
     * This class is responsible for creating and managing the Vulkan logical device, which is used to interact with 
     * the physical device. It also provides methods to retrieve the graphics and present queues.
     */
    class LogicalDevice {
       public:

        LogicalDevice(Window& window, Instance& instance, Surface& surface, PhysicalDevice& physicalDevice);
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