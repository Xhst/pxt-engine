#include "engine.hpp"

#include "core/memory.hpp"
#include "core/events/event_dispatcher.hpp"
#include "core/events/window_event.hpp"
#include "scene/ecs/component.hpp"
#include "scene/ecs/entity.hpp"
#include "graphics/render_systems/simple_render_system.hpp"
#include "scene/script/script.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace CGEngine {

    class TestScript : public Script {
    public:
        void onUpdate(float deltaTime) override {
            auto& transform = get<TransformComponent>();
            transform.rotation.y = glm::mod(transform.rotation.y + 0.01f, glm::two_pi<float>());
            transform.rotation.x = glm::mod(transform.rotation.x + 0.01f, glm::two_pi<float>());
        }
    };

    Engine::Engine() {
        loadScene();
    }

    Engine::~Engine() {}

    void Engine::run() {
        SimpleRenderSystem simpleRenderSystem(m_device, m_renderer.getSwapChainRenderPass());

        m_window.setEventCallback(std::bind(&Engine::onEvent, this, std::placeholders::_1));
        
        m_scene.onStart();
        
        while (isRunning()) {
            glfwPollEvents();
            
            if (auto commandBuffer = m_renderer.beginFrame()) {
                m_renderer.beginSwapChainRenderPass(commandBuffer);
                m_scene.onUpdate(0.0f);
                simpleRenderSystem.renderScene(commandBuffer, m_scene);
                m_renderer.endSwapChainRenderPass(commandBuffer);
                m_renderer.endFrame();
            }
        }

        vkDeviceWaitIdle(m_device.device());
    }

    bool Engine::isRunning() {
        return !m_window.shouldClose() && m_running;
    }

    void Engine::onEvent(Event& event) {
        EventDispatcher dispatcher(event);

        dispatcher.dispatch<WindowCloseEvent>([this](auto& event) {
            m_running = false;
        });
    }

    // temporary helper function, creates a 1x1x1 cube centered at offset
    Shared<Model> createCubeModel(Device& device, glm::vec3 offset) {
        std::vector<Model::Vertex> vertices{

            // left face (white)
            {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
            {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
            {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

            // right face (yellow)
            {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
            {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
            {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .8f, .1f}},

            // top face (orange, remember y axis points down)
            {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
            {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
            {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

            // bottom face (red)
            {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
            {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
            {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .1f, .1f}},

            // nose face (blue)
            {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
            {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
            {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

            // tail face (green)
            {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
            {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
            {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},

        };
        for (auto& v : vertices) {
            v.position += offset;
        }
        return createShared<Model>(device, vertices);
    }

    void Engine::loadScene() {
        
        auto model = createCubeModel(m_device, glm::vec3{0.0f, 0.0f, 0.0f});

        Entity entity = m_scene.createEntity("cube")
            .add<TransformComponent>(glm::vec3{0.0f, 0.0f, .5f}, glm::vec3{0.5f, 0.5f, .5f}, glm::vec3{0.0f, 0.0f, 0.0f})
            .add<ColorComponent>(glm::vec3{0.6f, 0.0f, 0.5f})
            .add<ModelComponent>(model);

        entity.addAndGet<ScriptComponent>().bind<TestScript>();
    }
}