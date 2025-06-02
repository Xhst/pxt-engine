#include "pxtengine.h"

#include "camera_controller.hpp"
#include "rotating_light_controller.hpp"

#include <random>

using namespace PXTEngine;

class App : public Application {
public:
    App() : Application() {}

    void prepareEnvironment() {
        std::array<std::string, 6> skyboxTextures;
        skyboxTextures[CubeFace::BACK] = TEXTURES_PATH + "skybox/bluecloud_bk.jpg";
        skyboxTextures[CubeFace::FRONT] = TEXTURES_PATH + "skybox/bluecloud_ft.jpg";
        skyboxTextures[CubeFace::LEFT] = TEXTURES_PATH + "skybox/bluecloud_lf.jpg";
        skyboxTextures[CubeFace::RIGHT] = TEXTURES_PATH + "skybox/bluecloud_rt.jpg";
        skyboxTextures[CubeFace::TOP] = TEXTURES_PATH + "skybox/bluecloud_up.jpg";
        skyboxTextures[CubeFace::BOTTOM] = TEXTURES_PATH + "skybox/bluecloud_dn.jpg";

        auto environment = getScene().getEnvironment();

        environment->setAmbientLight({ 0.67f, 0.85f, 0.9f, 0.25f });
        environment->setSkybox(skyboxTextures);
    }

    void createCameraEntity() {
        Entity camera = getScene().createEntity("camera")
            .add<TransformComponent>(glm::vec3{ 0.0f, -0.2f, -1.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ -glm::pi<float>() / 4, 0.0f, 0.0f })
            .add<CameraComponent>();

        camera.addAndGet<ScriptComponent>().bind<CameraController>();
	}

    Entity createPointLightEntity(const float intensity = 1.0f,
                            const float radius = 0.1f,
                            const glm::vec3 color = glm::vec3(1.f)) {
        Entity entity = getScene().createEntity("point_light")
            .add<PointLightComponent>(intensity)
            .add<TransformComponent>(glm::vec3{ 0.f, 0.f, 0.f }, glm::vec3{ radius, 1.f, 1.f }, glm::vec3{ 0.0f, 0.0f, 0.0f })
            .add<ColorComponent>(color);

        return entity;
    }

    void createFloor() {
        auto& rm = getResourceManager();

        ImageInfo albedoInfo{};
        albedoInfo.format = RGBA8_SRGB;

        auto ground = rm.get<Mesh>(MODELS_PATH + "quad.obj");
		auto stylizedStoneMaterial = Material::Builder()
			.setAlbedoMap(rm.get<Image>(TEXTURES_PATH + "stylized_stone/base.png", &albedoInfo))
			.setNormalMap(rm.get<Image>(TEXTURES_PATH + "stylized_stone/normal.png"))
			.setRoughnessMap(rm.get<Image>(TEXTURES_PATH + "stylized_stone/roughness.png"))
			.setAmbientOcclusionMap(rm.get<Image>(TEXTURES_PATH + "stylized_stone/ambient_occlusion.png"))
			.build();
		rm.add(stylizedStoneMaterial, "stylized_stone_material");

        Entity entity = getScene().createEntity("Floor")
            .add<TransformComponent>(glm::vec3{0.f, 1.f, 0.f}, glm::vec3{15.f, 15.f, 15.f}, glm::vec3{0.0f, 0.0f, 0.0f})
            .add<MeshComponent>(ground)
			.add<MaterialComponent>(MaterialComponent::Builder()
				.setMaterial(stylizedStoneMaterial)
                .setTilingFactor(50.0f)
				.build());
    }

    void createVasesWithRandomTransforms(int count) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> posDist(-0.7f, 0.7f);
        std::uniform_real_distribution<float> scaleDist(0.35f, 1.0f);
        std::uniform_real_distribution<float> rotDist(0.0f, glm::two_pi<float>());

        auto& rm = getResourceManager();
        auto vaseMesh = rm.get<Mesh>(MODELS_PATH + "smooth_vase.obj");

        for (int i = 0; i < count; ++i) {
            glm::vec3 pos = { posDist(gen), posDist(gen), posDist(gen) };
            float uniformScale = scaleDist(gen);
            glm::vec3 scale = { uniformScale, uniformScale, uniformScale };
            glm::vec3 rotation = { rotDist(gen), rotDist(gen), rotDist(gen) };

            Entity entity = getScene().createEntity("vase" + std::to_string(i))
                .add<TransformComponent>(pos, scale, rotation)
                .add<MeshComponent>(vaseMesh);
            entity.addAndGet<MaterialComponent>().tint = glm::vec3(0.1f, 0.3f, 0.9f);
        }
	}

    void createLights() {
        //entity = createPointLightEntity(0.25f, 0.02f, glm::vec3{1.f, 1.f, 1.f});
        //entity.get<TransformComponent>().translation = glm::vec3{0.0f, 0.0f, 0.0f};

        // Three rotating lights (white, green, blue)
        Entity entity = createPointLightEntity(0.1f, 0.025f, glm::vec3{ 1.f, 1.f, 1.f });
        entity.get<TransformComponent>().translation = glm::vec3{ 10.0f / (float) sqrt(3), 0.5f, 0.2f };
        entity.addAndGet<ScriptComponent>().bind<RotatingLightController>();
#if 0
        entity = createPointLightEntity(0.1f, 0.025f, glm::vec3{ 0.f, 1.f, 0.f });
        entity.get<TransformComponent>().translation = glm::vec3{ -1.0f / (float) (2.0f * sqrt(3)), 0.2f, 0.5f };
        entity.addAndGet<ScriptComponent>().bind<RotatingLightController>();

        entity = createPointLightEntity(0.1f, 0.025f, glm::vec3{ 0.f, 0.f, 1.f });
        entity.get<TransformComponent>().translation = glm::vec3{ -1.0f / (float) (2.0f * sqrt(3)), 0.2f, -0.5f };
        entity.addAndGet<ScriptComponent>().bind<RotatingLightController>();
#endif
    }

    void loadScene() override {
		prepareEnvironment();
        createCameraEntity();
        createFloor();
        createVasesWithRandomTransforms(5);
        createLights();

        auto& rm = getResourceManager();

        ImageInfo albedoInfo{};
        albedoInfo.format = RGBA8_SRGB;

        auto bunny = rm.get<Mesh>(MODELS_PATH + "bunny/bunny.obj");
        auto bunnyMaterial = Material::Builder()
            .setAlbedoMap(rm.get<Image>(MODELS_PATH + "bunny/terracotta.jpg", &albedoInfo))
            .setNormalMap(rm.get<Image>(NORMAL_PIXEL_LINEAR))
			.setAmbientOcclusionMap(rm.get<Image>(WHITE_PIXEL_LINEAR))
            .build();
		rm.add(bunnyMaterial, "bunny_material");

        Entity entity = getScene().createEntity("Bunny")
            .add<TransformComponent>(glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec3{ 2.5f, 2.5f, 2.5f }, glm::vec3{ glm::pi<float>(), 0.0f, 0.0f })
            .add<MeshComponent>(bunny)
            .add<MaterialComponent>(MaterialComponent::Builder()
                .setMaterial(bunnyMaterial)
                .build());
    }

    
};

PXTEngine::Application* PXTEngine::initApplication() {
    return new App();
}