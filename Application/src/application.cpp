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
            .add<TransformComponent>(glm::vec3{0.0f, -0.2f, -0.5f}, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 0.0f})
            .add<CameraComponent>();
        
        camera.addAndGet<ScriptComponent>().bind<CameraController>();

        Shared<Model> model_smooth_vase = Model::createModelFromFile(getDevice(), "../assets/models/smooth_vase.obj");
        Entity entity = getScene().createEntity("smooth_vase")
            .add<TransformComponent>(glm::vec3{-0.2f, 0.f, 0.f}, glm::vec3{0.5f, 0.25f, .5f}, glm::vec3{0.0f, 0.0f, 0.0f})
            .add<MaterialComponent>(glm::vec3{0.6f, 0.0f, 0.5f})
            .add<ModelComponent>(model_smooth_vase);

        Shared<Model> model_bunny = Model::createModelFromFile(getDevice(), "../assets/models/stanford_bunny.obj");
        entity = getScene().createEntity("bunny")
            .add<TransformComponent>(glm::vec3{0.0f, 0.f, 0.f}, glm::vec3{0.5f, 0.5f, .5f}, glm::vec3{0.0f, glm::pi<float>(), 0.0f})
            .add<MaterialComponent>(glm::vec4{1.6f, 1.0f, 1.0f, 1.0f}, 1.0f, 50.0f)
            .add<ModelComponent>(model_bunny);

        Shared<Model> model_pawn = Model::createModelFromFile(getDevice(), "../assets/models/pawn.obj");
        entity = getScene().createEntity("pawn")
            .add<TransformComponent>(glm::vec3{0.0f, 0.f, 2.5f}, glm::vec3{0.5f, 0.5f, .5f}, glm::vec3{0.0f, glm::pi<float>(), 0.0f})
            .add<MaterialComponent>(glm::vec3{1.0f, 1.0f, 1.0f})
            .add<ModelComponent>(model_pawn);

        Shared<Model> model_flat_vase = Model::createModelFromFile(getDevice(), "../assets/models/flat_vase.obj");
        entity = getScene().createEntity("flat_vase")
            .add<TransformComponent>(glm::vec3{0.2f, 0.f, 0.f}, glm::vec3{0.5f, 0.25f, .5f}, glm::vec3{0.0f, 0.0f, 0.0f})
            .add<MaterialComponent>(glm::vec3{1.0f, 1.0f, 1.0f})
            .add<ModelComponent>(model_flat_vase);

        Shared<Model> model_floor = Model::createModelFromFile(getDevice(), "../assets/models/quad.obj");
        entity = getScene().createEntity("floor")
            .add<TransformComponent>(glm::vec3{0.f, 0.f, 0.f}, glm::vec3{10.f, 1.f, 10.f}, glm::vec3{0.0f, 0.0f, 0.0f})
            .add<MaterialComponent>(glm::vec3{1.0f, 1.0f, 1.0f})
            .add<ModelComponent>(model_floor);

        entity = createPointLight(1.0f, 0.1f, glm::vec3{1.f, 0.f, 0.f});
        entity.get<TransformComponent>().translation = glm::vec3{1.f, -1.f, 1.5f};

        entity = createPointLight(1.7f, 0.35f, glm::vec3{0.f, 0.f, 1.f});
        entity.get<TransformComponent>().translation = glm::vec3{-1.f, -1.f, 0.5f};

        entity = createPointLight(8.f, 0.5f, glm::vec3{0.8f, 0.9f, 0.05f});
        entity.get<TransformComponent>().translation = glm::vec3{0.8f, -2.f, 5.0f};
    }

    
};

CGEngine::Application* CGEngine::initApplication() {
    return new App();
}