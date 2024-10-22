#include "application.hpp"

#include "core/memory.hpp"
#include "core/events/event_dispatcher.hpp"
#include "core/events/window_event.hpp"
#include "scene/ecs/component.hpp"
#include "scene/ecs/entity.hpp"
#include "scene/camera.hpp"
#include "graphics/render_systems/simple_render_system.hpp"
#include "scene/script/script.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <chrono>

namespace CGEngine {

    struct GlobalUbo {
        glm::mat4 projectionView{1.f};
        glm::vec3 lightDirection = glm::normalize(glm::vec3(1.f, -3.f, -1.f));
    };

    Application* Application::Instance = nullptr;

    Application::Application() {
        Instance = this;
    }

    void Application::run() {
        Buffer globalUboBuffer{
            m_device,
            sizeof(GlobalUbo),
            SwapChain::MAX_FRAMES_IN_FLIGHT,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            m_device.properties.limits.minUniformBufferOffsetAlignment
        };
        globalUboBuffer.map();

        m_window.setEventCallback(std::bind(&Application::onEvent, this, std::placeholders::_1));

        SimpleRenderSystem simpleRenderSystem(m_device, m_renderer.getSwapChainRenderPass());
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
                camera.setPerspective(glm::radians(50.f), aspect, 0.1f, 100.f);
                camera.setViewYXZ(transform.translation, transform.rotation);
            }
            
            if (auto commandBuffer = m_renderer.beginFrame()) {
                int frameIndex = m_renderer.getFrameIndex();

                FrameInfo frameInfo = {
                    frameIndex,
                    elapsedTime,
                    commandBuffer,
                    camera
                };

                // update
                GlobalUbo globalUbo{};
                globalUbo.projectionView = camera.getProjectionMatrix() * camera.getViewMatrix();
                globalUboBuffer.writeToBuffer(&globalUbo, frameIndex);
                
                for (auto& [_, system] : m_systems) {
                    system->onUpdate(elapsedTime);
                }

                // render 
                m_renderer.beginSwapChainRenderPass(commandBuffer);

                simpleRenderSystem.renderScene(frameInfo, m_scene);

                m_renderer.endSwapChainRenderPass(commandBuffer);
                m_renderer.endFrame();
            }
        }

        vkDeviceWaitIdle(m_device.device());
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
    
}

int main() {

    try {
        auto app = CGEngine::initApplication();

        app->run();

        delete app;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}