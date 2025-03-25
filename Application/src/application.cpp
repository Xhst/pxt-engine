#include "pxtengine.h"

#include "camera_controller.hpp"
#include "rotating_light_controller.hpp"

#include <iostream>

using namespace PXTEngine;

class App : public Application {
public:
    App() : Application() {
        loadScene();
    }

    ~App() {

    }

    void loadScene() {
        Entity camera = getScene().createEntity("camera")
            .add<TransformComponent>(glm::vec3{0.0f, -1.2f, -1.0f}, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{-glm::pi<float>()/4, 0.0f, 0.0f})
            .add<CameraComponent>();
        
        camera.addAndGet<ScriptComponent>().bind<CameraController>();

        Shared<Model> quad = Model::createModelFromFile(getDevice(), MODELS_PATH + "quad.obj");
        Entity entity = getScene().createEntity("Floor")
            .add<TransformComponent>(glm::vec3{0.f, 0.f, 0.f}, glm::vec3{1.f, 1.f, 1.f}, glm::vec3{0.0f, 0.0f, 0.0f})
            .add<MaterialComponent>(glm::vec3{1.0f, 1.0f, 1.0f})
            .add<ModelComponent>(quad);

        entity = getScene().createEntity("Roof")
            .add<TransformComponent>(glm::vec3{0.f, -2.0f, 0.f}, glm::vec3{1.f, 1.f, 1.f}, glm::vec3{0.0f, 0.0f, 0.0f})
            .add<MaterialComponent>(glm::vec3{1.0f, 1.0f, 1.0f})
            .add<ModelComponent>(quad);

        entity = getScene().createEntity("BackWall")
            .add<TransformComponent>(glm::vec3{0.0f, -1.f, 1.f}, glm::vec3{1.f, 1.f, 1.f}, glm::vec3{glm::pi<float>()/2, 0.0f, 0.0f})
            .add<MaterialComponent>(glm::vec3{1.0f, 1.0f, 1.0f})
            .add<ModelComponent>(quad);

            entity = getScene().createEntity("FrontWall")
            .add<TransformComponent>(glm::vec3{0.0f, -1.f, -1.f}, glm::vec3{1.f, 1.f, 1.f}, glm::vec3{-glm::pi<float>()/2, 0.0f, 0.0f})
            .add<MaterialComponent>(glm::vec3{1.0f, 1.0f, 1.0f})
            .add<ModelComponent>(quad);

        entity = getScene().createEntity("RightWall")
            .add<TransformComponent>(glm::vec3{1.0f, -1.f, 0.0f}, glm::vec3{1.f, 1.f, 1.f}, glm::vec3{glm::pi<float>()/2, glm::pi<float>()/2, 0.0f})
            .add<MaterialComponent>(glm::vec3{1.0f, 1.0f, 1.0f})
            .add<ModelComponent>(quad);
        
        entity = getScene().createEntity("LeftWall")
            .add<TransformComponent>(glm::vec3{-1.0f, -1.f, 0.0f}, glm::vec3{1.f, 1.f, 1.f}, glm::vec3{glm::pi<float>()/2, -glm::pi<float>()/2, 0.0f})
            .add<MaterialComponent>(glm::vec3{1.0f, 1.0f, 1.0f})
            .add<ModelComponent>(quad);

        glm::vec4 colorWhite = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        glm::vec4 colorGold = glm::vec4{1.0f, 0.843f, 0.0f, 1.0f};

        // Bunny
        Shared<Model> model_bunny = Model::createModelFromFile(getDevice(), MODELS_PATH + "stanford_bunny.obj");
        entity = getScene().createEntity("bunny")
            .add<TransformComponent>(glm::vec3{0.0f, 0.f, 0.f}, glm::vec3{1.0f, 1.0f, 1.0}, glm::vec3{0.0f, glm::pi<float>(), 0.0f})
            .add<ModelComponent>(model_bunny)
            //.add<MaterialComponent>(colorWhite)
            .add<MaterialComponent>(colorGold, 2.0f, 140.0f);

        // Light above the bunny
        entity = createPointLight(0.025f, 0.02f, glm::vec3{1.f, 1.f, 1.f});
        entity.get<TransformComponent>().translation = glm::vec3{0.0f, -0.65f, 0.0f};

        // Three rotating lights around the bunny (red, green, blue)
        entity = createPointLight(0.1f, 0.025f, glm::vec3{1.f, 0.f, 0.f});
        entity.get<TransformComponent>().translation = glm::vec3{1.0f / (float) sqrt(3), -0.5f, 0.0f};
        entity.addAndGet<ScriptComponent>().bind<RotatingLightController>();

        entity = createPointLight(0.1f, 0.025f, glm::vec3{0.f, 1.f, 0.f});
        entity.get<TransformComponent>().translation = glm::vec3{-1.0f / (float) (2.0f * sqrt(3)), -0.5f, 0.5f};
        entity.addAndGet<ScriptComponent>().bind<RotatingLightController>();

        entity = createPointLight(0.1f, 0.025f, glm::vec3{0.f, 0.f, 1.f});
        entity.get<TransformComponent>().translation = glm::vec3{-1.0f / (float) (2.0f * sqrt(3)), -0.5f, -0.5f};
        entity.addAndGet<ScriptComponent>().bind<RotatingLightController>();
        
    }

    
};

PXTEngine::Application* PXTEngine::initApplication() {
    return new App();
}