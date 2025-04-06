#pragma once

#include "core/memory.hpp"
#include "core/uuid.hpp"
#include "core/system.hpp"
#include "core/events/event.hpp"
#include "graphics/window.hpp"
#include "graphics/context/context.hpp"
#include "graphics/renderer.hpp"
#include "graphics/descriptors/descriptors.hpp"
#include "graphics/frame_info.hpp"
#include "scene/scene.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

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

        Context& getContext() {
            return m_context;
        }

        Window& getWindow() {
            return m_window;
        }

        Entity createPointLight(const float intensity = 1.0f, const float radius = 0.1f, const glm::vec3 color = glm::vec3(1.f));
    
    private:
        void run();
        void onEvent(Event& event);
        bool isRunning();

        void initImGui();

        void imGuiRenderUI(FrameInfo& frameInfo);

        bool m_running = true;

        Window m_window{WindowData()};
        Context m_context{m_window};
        Renderer m_renderer{m_window, m_context};

        Unique<DescriptorAllocatorGrowable> m_descriptorAllocator{};
        Unique<DescriptorPool> m_globalPool{};
        Unique<DescriptorPool> m_imGuiPool{};

        Scene m_scene{};
        std::unordered_map<UUID, System*> m_systems;

        static Application* Instance;

        friend int ::main();
    };

    Application* initApplication();
}