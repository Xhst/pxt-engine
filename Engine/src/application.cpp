#include "application.hpp"

#include "core/memory.hpp"
#include "core/buffer.hpp"
#include "core/constants.hpp"
#include "core/events/event_dispatcher.hpp"
#include "core/events/window_event.hpp"
#include "core/error_handling.hpp"
#include "scene/ecs/component.hpp"
#include "scene/ecs/entity.hpp"
#include "scene/camera.hpp"
#include "graphics/render_systems/master_render_system.hpp"
#include "graphics/resources/texture2d.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <iostream>
#include <chrono>

namespace PXTEngine {

    Application* Application::m_instance = nullptr;

    Application::Application() {
        m_instance = this;
    }

    Application::~Application() {};

    void Application::start() {
        createDescriptorPoolAllocator();
        createUboBuffers();
        createGlobalDescriptorSet();

        createDefaultResources();
        loadScene();
        registerImages();
        m_textureRegistry.setDescriptorAllocator(m_descriptorAllocator);
		m_textureRegistry.createDescriptorSet();

        m_masterRenderSystem = createUnique<MasterRenderSystem>(
            m_context,
            m_renderer,
            m_descriptorAllocator,
            m_textureRegistry,
            m_globalSetLayout
        );

        m_window.setEventCallback([this]<typename E>(E && event) {
            onEvent(std::forward<E>(event));
        });
    }

	void Application::createDescriptorPoolAllocator() {
		std::vector<PoolSizeRatio> ratios = {
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1.0f},
			{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5.0f},
		};

		m_descriptorAllocator = createShared<DescriptorAllocatorGrowable>(m_context, SwapChain::MAX_FRAMES_IN_FLIGHT, ratios);
	}

	void Application::createUboBuffers() {
		for (int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
			m_uboBuffers[i] = createUnique<VulkanBuffer>(
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

    void Application::createDefaultResources() {
        // color are stored in RGBA format but bytes are reversed (Little-Endian Systems)
        // 0x0A0B0C0D -> Alpha = 0A, Blue = 0B, Green = 0C, Red = 0D
        std::unordered_map<std::string, std::pair<uint32_t, ImageFormat>> defaultImagesData = {
            {WHITE_PIXEL, {0xFFFFFFFF, RGBA8_SRGB} },
            {WHITE_PIXEL_LINEAR, {0xFFFFFFFF, RGBA8_LINEAR} },
			{GRAY_PIXEL_LINEAR, {0xFF808080, RGBA8_LINEAR} },
            {BLACK_PIXEL_LINEAR, {0xFF000000, RGBA8_LINEAR} },
            {NORMAL_PIXEL_LINEAR, {0xFFFF8080, RGBA8_LINEAR} }
        };

        for (const auto& [name, data] : defaultImagesData) {
            // Create a buffer with the pixel data
            auto color = data.first;

            ImageInfo info;
            info.width = 1;
            info.height = 1;
            info.channels = 4;
            info.format = data.second;

            Buffer buffer = Buffer(&color, sizeof(color));
            Shared<Image> image = createShared<Texture2D>(m_context, info, buffer);
            m_resourceManager.add(image, name);
        }

        auto defaultMaterial = Material::Builder()
            .setAlbedoColor(glm::vec4(1.0f))
            .setAlbedoMap(m_resourceManager.get<Image>(WHITE_PIXEL))
            .setNormalMap(m_resourceManager.get<Image>(NORMAL_PIXEL_LINEAR))
            .setAmbientOcclusionMap(m_resourceManager.get<Image>(WHITE_PIXEL_LINEAR))
            .setMetallicMap(m_resourceManager.get<Image>(BLACK_PIXEL_LINEAR))
            .setRoughnessMap(m_resourceManager.get<Image>(GRAY_PIXEL_LINEAR))
            .build();

		m_resourceManager.add(defaultMaterial, DEFAULT_MATERIAL);
    }

    void Application::registerImages() {
		// iterate over resource and register images
		m_resourceManager.foreach([&](const Shared<Resource>& resource) {
			if (resource->getType() != Resource::Type::Image) return;

			const auto image = std::static_pointer_cast<Image>(resource);
			m_textureRegistry.add(image);
		});
    }


    void Application::run() {
        Camera camera;
        
        auto currentTime = std::chrono::high_resolution_clock::now();
    
        m_scene.onStart();
        
        while (isRunning()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float elapsedTime = std::chrono::duration<float>(newTime - currentTime).count();
            currentTime = newTime;
            
            m_scene.onUpdate(elapsedTime);

            updateCamera(camera);
            
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
    }

    void Application::updateCamera(Camera& camera)
	{
        if (Entity mainCameraEntity = m_scene.getMainCameraEntity()) {
            const auto& cameraComponent = mainCameraEntity.get<CameraComponent>();
            const auto& transform = mainCameraEntity.get<TransformComponent>();

            camera = cameraComponent.camera;
            camera.setViewYXZ(transform.translation, transform.rotation);

			//TODO: camera projection
            camera.setPerspective(
                glm::radians(50.f), 
                m_renderer.getAspectRatio(), 
                0.1f, 100.f);
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

        app->start();
        app->run();

        delete app;
    } catch (const std::exception& e) {
		PXT_ERROR("Application crashed: {}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}