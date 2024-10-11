#include "cgengine.h"

#include <iostream>

using namespace CGEngine;

class TestScript : public Script {
    public:
        void onUpdate(float deltaTime) override {
            auto& transform = get<TransformComponent>();
            //transform.rotation.y = glm::mod(transform.rotation.y + 0.001f, glm::two_pi<float>());
            //transform.rotation.x = glm::mod(transform.rotation.x + 0.0001f, glm::two_pi<float>());
        }
};

class CameraController : public Script {
    public:
        void onUpdate(float deltaTime) override {
            float moveSpeed{3.f};
            float lookSpeed{1.5f};

            auto& transform = get<TransformComponent>();

            glm::vec3 rotate{0};
            if (Input::isKeyPressed(KeyCode::RightArrow)) rotate.y += 1.f;
            if (Input::isKeyPressed(KeyCode::LeftArrow)) rotate.y -= 1.f;
            if (Input::isKeyPressed(KeyCode::UpArrow)) rotate.x += 1.f;
            if (Input::isKeyPressed(KeyCode::DownArrow)) rotate.x -= 1.f;

            if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
                transform.rotation += lookSpeed * deltaTime * glm::normalize(rotate);
            }

            transform.rotation.x = glm::clamp(transform.rotation.x, -1.5f, 1.5f);
            transform.rotation.y = glm::mod(transform.rotation.y, glm::two_pi<float>());

            float yaw = transform.rotation.y;
            const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
            const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
            const glm::vec3 upDir{0.f, -1.f, 0.f};

            glm::vec3 moveDir{0.f};
            if (Input::isKeyPressed(KeyCode::W)) moveDir += forwardDir;
            if (Input::isKeyPressed(KeyCode::S)) moveDir -= forwardDir;
            if (Input::isKeyPressed(KeyCode::D)) moveDir += rightDir;
            if (Input::isKeyPressed(KeyCode::A)) moveDir -= rightDir;
            if (Input::isKeyPressed(KeyCode::E)) moveDir += upDir;
            if (Input::isKeyPressed(KeyCode::Q)) moveDir -= upDir;

            std::cout << "moveDir: " << moveDir.x << ", " << moveDir.y << ", " << moveDir.z << std::endl;

            if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
                transform.translation += moveSpeed * deltaTime * glm::normalize(moveDir);
            }
        }
};

class App : public Application {
    public:
        App() : Application() {
            loadScene();
        }

        ~App() {

        }

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

    void loadScene() {
        Entity camera = getScene().createEntity("camera")
            .add<TransformComponent>(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 0.0f})
            .add<CameraComponent>();
        
        camera.addAndGet<ScriptComponent>().bind<CameraController>();

        auto model = createCubeModel(getDevice(), glm::vec3{0.0f, 0.0f, 0.0f});
        Entity entity = getScene().createEntity("cube")
            .add<TransformComponent>(glm::vec3{0.0f, 0.0f, 2.5f}, glm::vec3{0.5f, 0.5f, .5f}, glm::vec3{0.0f, 0.0f, 0.0f})
            .add<ColorComponent>(glm::vec3{0.6f, 0.0f, 0.5f})
            .add<ModelComponent>(model);

        entity.addAndGet<ScriptComponent>().bind<TestScript>();
    }
};

CGEngine::Application* CGEngine::initApplication() {
    return new App();
}