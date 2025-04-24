#pragma once

#include "core/memory.hpp"
#include "core/uuid.hpp"
#include "core/events/event.hpp"
#include "graphics/window.hpp"
#include "graphics/context/context.hpp"
#include "graphics/renderer.hpp"
#include "graphics/descriptors/descriptors.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/render_systems/master_render_system.hpp"
#include "scene/scene.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

int main();

namespace PXTEngine {

    class Application {
    public:
        Application();
        virtual ~Application();

        static Application& get() { return *m_instance; }

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
		void createDescriptorPoolAllocator();
		void createUboBuffers();
        void createGlobalDescriptorSet();

        void run();
        void onEvent(Event& event);
        bool isRunning();
		void updateCamera(Camera& camera);

        bool m_running = true;

        Window m_window{WindowData()};
        Context m_context{m_window};
        Renderer m_renderer{m_window, m_context};
        Unique<MasterRenderSystem> m_masterRenderSystem;

		Shared<DescriptorAllocatorGrowable> m_descriptorAllocator{};
		Shared<DescriptorSetLayout> m_globalSetLayout{};
		std::vector<VkDescriptorSet> m_globalDescriptorSets{ SwapChain::MAX_FRAMES_IN_FLIGHT };

		std::vector<Unique<VulkanBuffer>> m_uboBuffers{ SwapChain::MAX_FRAMES_IN_FLIGHT };

        Scene m_scene{};

        static Application* m_instance;

        friend int ::main();
    };

    Application* initApplication();
}