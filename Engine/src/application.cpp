#include "application.hpp"

#include "core/memory.hpp"
#include "core/events/event_dispatcher.hpp"
#include "core/events/window_event.hpp"
#include "core/constants.hpp"
#include "scene/ecs/component.hpp"
#include "scene/ecs/entity.hpp"
#include "scene/camera.hpp"
#include "graphics/resources/image.hpp"
#include "graphics/render_systems/master_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <iostream>
#include <chrono>

namespace PXTEngine {

    Application* Application::Instance = nullptr;

    Application::Application() {
        Instance = this;

		createDescriptorPoolAllocator();
		createUboBuffers();
		createGlobalDescriptorSet();

		m_masterRenderSystem = createUnique<MasterRenderSystem>(
			m_context,
			m_renderer,
			m_descriptorAllocator,
            m_globalSetLayout
		);
    }

    Application::~Application() {
            for (auto& [_, system] : m_systems) {
                system->onShutdown();
                delete system;
            }
        };

	void Application::createDescriptorPoolAllocator() {
		std::vector<PoolSizeRatio> ratios = {
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1.0f},
			{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5.0f},
		};

		m_descriptorAllocator = createShared<DescriptorAllocatorGrowable>(m_context, SwapChain::MAX_FRAMES_IN_FLIGHT, ratios);
	}

	void Application::createUboBuffers() {
		for (int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
			m_uboBuffers[i] = createUnique<Buffer>(
				m_context,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			);
			m_uboBuffers[i]->map();
		}
	}

    void Application::createGlobalDescriptorSet() {
        m_globalSetLayout = DescriptorSetLayout::Builder(m_context)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

        for (int i = 0; i < m_globalDescriptorSets.size(); i++) {
            auto bufferInfo = m_uboBuffers[i]->descriptorInfo();

            m_descriptorAllocator->allocate(m_globalSetLayout->getDescriptorSetLayout(), m_globalDescriptorSets[i]);

            DescriptorWriter(m_context, *m_globalSetLayout)
                .writeBuffer(0, &bufferInfo)
				.updateSet(m_globalDescriptorSets[i]);
        }
    }

    void Application::run() {

        m_window.setEventCallback([this](auto&& PH1) {
	        onEvent(std::forward<decltype(PH1)>(PH1));
        });

        Camera camera;
        
        auto currentTime = std::chrono::high_resolution_clock::now();
    
        m_scene.onStart();
        
        while (isRunning()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float elapsedTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;
            
            m_scene.onUpdate(elapsedTime);
            
            float aspect = m_renderer.getAspectRatio();

            Entity mainCameraEntity = m_scene.getMainCameraEntity();

            if (mainCameraEntity) {
                const auto& cameraComponent = mainCameraEntity.get<CameraComponent>();
                const auto& transform = mainCameraEntity.get<TransformComponent>();

                camera = cameraComponent.camera;
                camera.setViewYXZ(transform.translation, transform.rotation);

                camera.setPerspective(glm::radians(50.f), aspect, 0.1f, 100.f);
            }
            
            if (auto commandBuffer = m_renderer.beginFrame()) {
                int frameIndex = m_renderer.getFrameIndex();

                FrameInfo frameInfo = {
                    frameIndex,
                    elapsedTime,
                    commandBuffer,
                    camera,
                    m_globalDescriptorSets[frameIndex],
                    m_scene
                };

				// its not used yet
				for (auto& [_, system] : m_systems) {
					system->onUpdate(elapsedTime);
				}

                GlobalUbo ubo{};

				m_masterRenderSystem->onUpdate(frameInfo, ubo);

				m_uboBuffers[frameIndex]->writeToBuffer(&ubo);
				m_uboBuffers[frameIndex]->flush();

				m_masterRenderSystem->doRenderPasses(frameInfo);

                m_renderer.endFrame();
            }
        }

        vkDeviceWaitIdle(m_context.getDevice());
    }

    bool Application::isRunning() {
        return !m_window.shouldClose() && m_running;
    }

    void Application::onEvent(Event& event) {
        EventDispatcher dispatcher(event);

        dispatcher.dispatch<WindowCloseEvent>([this](auto& event) {
            m_running = false;
        });

        for (auto& [_, system] : m_systems) {
            if (event.isHandled()) {
                break;
            }

            system->onEvent(event);
        }
    }

    Entity Application::createPointLight(float intensity, float radius, glm::vec3 color) {
        Entity entity = m_scene.createEntity("point_light")
            .add<PointLightComponent>(intensity)
            .add<TransformComponent>(glm::vec3{0.f, 0.f, 0.f}, glm::vec3{radius, 1.f, 1.f}, glm::vec3{0.0f, 0.0f, 0.0f})
            .add<ColorComponent>(color);

        return entity;
    }
    
}

int main() {

    try {
        auto app = PXTEngine::initApplication();

        app->run();

        delete app;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}