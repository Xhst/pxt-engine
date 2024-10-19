#include "cgengine.h"

#include "camera_controller.hpp"

#include <iostream>

using namespace CGEngine;

class App : public Application {
public:
    App() : Application() {
        loadScene();
    }

    ~App() {

    }

    void loadScene() {
        Entity camera = getScene().createEntity("camera")
            .add<TransformComponent>(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 0.0f})
            .add<CameraComponent>();
        
        camera.addAndGet<ScriptComponent>().bind<CameraController>();

        Shared<Model> model = Model::createModelFromFile(getDevice(), "../assets/models/smooth_vase.obj");
        Entity entity = getScene().createEntity("object")
            .add<TransformComponent>(glm::vec3{0.0f, 0.0f, 2.5f}, glm::vec3{0.5f, 0.5f, .5f}, glm::vec3{0.0f, 0.0f, 0.0f})
            .add<ColorComponent>(glm::vec3{0.6f, 0.0f, 0.5f})
            .add<ModelComponent>(model);
    }
};

CGEngine::Application* CGEngine::initApplication() {
    return new App();
}