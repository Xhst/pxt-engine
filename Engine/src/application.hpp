#pragma once

#include "core/pch.hpp"
#include "core/events/event.hpp"
#include "graphics/window.hpp"
#include "graphics/context/context.hpp"
#include "graphics/renderer.hpp"
#include "graphics/descriptors/descriptors.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/render_systems/master_render_system.hpp"
#include "graphics/resources/texture_registry.hpp"
#include "graphics/resources/material_registry.hpp"
#include "graphics/resources/blas_registry.hpp"
#include "resources/resource_manager.hpp"
#include "resources/types/material.hpp"
#include "scene/scene.hpp"

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

        ResourceManager& getResourceManager() {
            return m_resourceManager;
        }

		Shared<DescriptorAllocatorGrowable> getDescriptorAllocator() {
			return m_descriptorAllocator;
		}

    protected:
        virtual void loadScene() {}
    private:
		void createDescriptorPoolAllocator();
		void createUboBuffers();
        void createGlobalDescriptorSet();
        void createDefaultResources();
        void registerResources();

        void start();
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

        ResourceManager m_resourceManager{};
        TextureRegistry m_textureRegistry{m_context};
		MaterialRegistry m_materialRegistry{m_context, m_textureRegistry};
		BLASRegistry m_blasRegistry{m_context};

        static Application* m_instance;

        friend int ::main();
    };

    Application* initApplication();
}