#pragma once

#include "core/memory.hpp"
#include "core/uuid.hpp"
#include "core/system.hpp"
#include "core/events/event.hpp"
#include "graphics/instance.hpp"
#include "graphics/window.hpp"
#include "graphics/device.hpp"
#include "graphics/renderer.hpp"
#include "graphics/descriptors/descriptors.hpp"
#include "graphics/frame_info.hpp"
#include "scene/scene.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <iostream>
#include <stdexcept>

int main();

namespace PXTEngine {

    class Application {
    public:
        Application();
        ~Application();

        static Application& get() { return *Instance; }

        void addSystem(System* system) {
            m_systems[system->getId()] = system;
            system->onInit();
        }

        void removeSystem(System* system) {
            system->onShutdown();
            m_systems.erase(system->getId());
        }

        Scene& getScene() {
            return m_scene;
        }

        Device& getDevice() {
            return m_device;
        }

        Window& getWindow() {
            return m_window;
        }

        Entity createPointLight(const float intensity = 1.0f, const float radius = 0.1f, const glm::vec3 color = glm::vec3(1.f));
    
    private:
        void run();
        void onEvent(Event& event);
        bool isRunning();

        void initImGui(Window& window, Device& device);

        void imGuiRenderUI(FrameInfo& frameInfo);

        bool m_running = true;

        Instance m_instance{"PXTEngine"};
        Window m_window{WindowData()};
        Device m_device{m_window};
        Renderer m_renderer{m_window, m_device};

        Unique<DescriptorPool> m_globalPool{};
        Scene m_scene{};

        std::unordered_map<UUID, System*> m_systems;

        static Application* Instance;

        friend int ::main();
    };

    Application* initApplication();
}