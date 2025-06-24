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

        environment->setAmbientLight({ 1.0, 1.0, 1.0, 0.15f });
        environment->setSkybox(skyboxTextures);
    }

    void createCameraEntity() {
        Entity camera = getScene().createEntity("camera")
            .add<TransformComponent>(glm::vec3{ -0.1f, -0.4f, -1.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ -glm::pi<float>() / 4, 0.0f, 0.0f })
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

        auto quad = rm.get<Mesh>(MODELS_PATH + "quad.obj");
		auto stylizedStoneMaterial = Material::Builder()
			.setAlbedoMap(rm.get<Image>(TEXTURES_PATH + "laminated_wood/albedo.png", &albedoInfo))
			.setNormalMap(rm.get<Image>(TEXTURES_PATH + "laminated_wood/normal.png"))
            .setMetallicMap(rm.get<Image>(TEXTURES_PATH + "laminated_wood/metallic.png"))
			.setRoughnessMap(rm.get<Image>(TEXTURES_PATH + "laminated_wood/roughness.png"))
			.setAmbientOcclusionMap(rm.get<Image>(TEXTURES_PATH + "laminated_wood/ao.png"))
			.build();
		rm.add(stylizedStoneMaterial, "floor_material");

        Entity entity = getScene().createEntity("Floor")
            .add<TransformComponent>(glm::vec3{0.f, 1.0f, 0.f}, glm::vec3{1.f, 1.f, 1.f}, glm::vec3{0.0f, 0.0f, 0.0f})
            .add<MeshComponent>(quad)
			.add<MaterialComponent>(MaterialComponent::Builder()
				.setMaterial(stylizedStoneMaterial)
                .setTilingFactor(2.0f)
				.build());

        entity = getScene().createEntity("Left Wall")
            .add<TransformComponent>(glm::vec3{ -1.f, 0.f, 0.f }, glm::vec3{ 1.f, 1.f, 1.f }, glm::vec3{ 0.0f, 0.0f, glm::pi<float>() / 2 })
            .add<MeshComponent>(quad);
        entity.addAndGet<MaterialComponent>().tint = glm::vec3{ 1.0f, 0.f, 0.f };

        entity = getScene().createEntity("Right Wall")
            .add<TransformComponent>(glm::vec3{ 1.f, 0.f, 0.f }, glm::vec3{ 1.f, 1.f, 1.f }, glm::vec3{ 0.0f, 0.0f, -glm::pi<float>() / 2 })
            .add<MeshComponent>(quad);
		entity.addAndGet<MaterialComponent>().tint = glm::vec3{ 0.f, 1.0f, 0.f };

        entity = getScene().createEntity("Front Wall")
            .add<TransformComponent>(glm::vec3{ 0.f, 0.f, 1.f }, glm::vec3{ 1.f, 1.f, 1.f }, glm::vec3{ glm::pi<float>() / 2, 0.0f, 0.0f })
            .add<MeshComponent>(quad)
            .add<MaterialComponent>();

        entity = getScene().createEntity("Roof")
            .add<TransformComponent>(glm::vec3{ 0.f, -1.f, 0.f }, glm::vec3{ 1.f, 1.f, 1.f }, glm::vec3{ glm::pi<float>(), 0.0f, 0.0f })
            .add<MeshComponent>(quad)
            .add<MaterialComponent>();
    }

    void createTeapotAndVases(int count) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> posDist(-0.7f, 0.7f);
        std::uniform_real_distribution<float> scaleDist(0.35f, 1.0f);
        std::uniform_real_distribution<float> rotDist(0.0f, glm::two_pi<float>());

        auto& rm = getResourceManager();
        auto vaseMesh = rm.get<Mesh>(MODELS_PATH + "smooth_vase.obj");
        auto teapotMesh = rm.get<Mesh>(MODELS_PATH + "utah_teapot.obj");

        ImageInfo albedoInfo{};
        albedoInfo.format = RGBA8_SRGB;

        auto metallicMaterial = Material::Builder()
            .setRoughnessMap(rm.get<Image>(TEXTURES_PATH + "/gold/roughness.png"))
            .setMetallicMap(rm.get<Image>(TEXTURES_PATH + "/gold/metallic.png"))
            .setNormalMap(rm.get<Image>(TEXTURES_PATH + "/gold/normal.png"))
            .build();
        rm.add(metallicMaterial, "metallic_material");

        auto graniteMaterial = Material::Builder()
            .setAlbedoMap(rm.get<Image>(TEXTURES_PATH + "granite/albedo.png", &albedoInfo))
            .setRoughnessMap(rm.get<Image>(TEXTURES_PATH + "granite/roughness.png"))
            .setMetallicMap(rm.get<Image>(TEXTURES_PATH + "granite/metallic.png"))
            .setNormalMap(rm.get<Image>(TEXTURES_PATH + "granite/normal.png"))
            .setAmbientOcclusionMap(rm.get<Image>(TEXTURES_PATH + "granite/ao.png"))
            .build();
        rm.add(graniteMaterial, "brown_granite");

        Entity entity = getScene().createEntity("vase")
            .add<TransformComponent>(glm::vec3{ -0.75f, 1.0f, 0.1f }, glm::vec3{ 1.0f, 1.0f, 1.0f }, glm::vec3{0.0f, glm::pi<float>()/4, 0.0f})
            .add<MeshComponent>(vaseMesh);
        entity.addAndGet<MaterialComponent>(MaterialComponent::Builder()
            .setMaterial(graniteMaterial).build());

        entity = getScene().createEntity("teapot")
            .add<TransformComponent>(glm::vec3{ 0.5f, 1.0f, 0.7f }, glm::vec3{ 0.15f, 0.15f, 0.15f }, glm::vec3{ glm::pi<float>(), -glm::pi<float>()/1.6, 0.0f })
            .add<MeshComponent>(teapotMesh);
        entity.addAndGet<MaterialComponent>(MaterialComponent::Builder()
            .setMaterial(metallicMaterial).build()).tint = glm::vec3(0.737, 0.776, 0.8);

        entity = getScene().createEntity("vase")
            .add<TransformComponent>(glm::vec3{ -0.65f, 1.0f, 0.4f }, glm::vec3{ 1.8f, 1.4f, 1.8f }, glm::vec3{ 0.0f, 0.0f, 0.0f })
            .add<MeshComponent>(vaseMesh);
        entity.addAndGet<MaterialComponent>(MaterialComponent::Builder()
            .setMaterial(graniteMaterial).build()).tint = glm::vec3(0.13f, 0.24f, 0.35f);
	}

    void createRubikCube() {
        auto& rm = getResourceManager();
        auto rubikMesh = rm.get<Mesh>(MODELS_PATH + "rubik.obj");

        ImageInfo albedoInfo{};
        albedoInfo.format = RGBA8_SRGB;

        auto rubikMaterial = Material::Builder()
            .setAlbedoMap(rm.get<Image>(TEXTURES_PATH + "/rubik/albedo.jpg", &albedoInfo))
            .setRoughnessMap(rm.get<Image>(TEXTURES_PATH + "/rubik/roughness.jpg"))
            .setNormalMap(rm.get<Image>(TEXTURES_PATH + "/rubik/normal.jpg"))
            .setAmbientOcclusionMap(rm.get<Image>(TEXTURES_PATH + "/rubik/ao.jpg"))
            .build();
        rm.add(rubikMaterial, "rubik_material");

        Entity entity = getScene().createEntity("rubik")
            .add<TransformComponent>(glm::vec3{ -0.75f, 0.9f, -0.3f }, glm::vec3{ 0.1f, 0.1f, 0.1f }, glm::vec3{ 0.0f, -glm::pi<float>()/2.5, 0.0f})
            .add<MeshComponent>(rubikMesh)
            .add<MaterialComponent>(MaterialComponent::Builder()
                .setMaterial(rubikMaterial).build());

    }

    void createLamp() {
		auto& rm = getResourceManager();

        ImageInfo albedoInfo{};
        albedoInfo.format = RGBA8_SRGB;

        auto lampMaterial = Material::Builder()
            .setAlbedoMap(rm.get<Image>(TEXTURES_PATH + "/lamp/albedo.png", &albedoInfo))
            .setRoughnessMap(rm.get<Image>(TEXTURES_PATH + "/lamp/roughness.png"))
            .setMetallicMap(rm.get<Image>(TEXTURES_PATH + "/lamp/metallic.png"))
            .setNormalMap(rm.get<Image>(TEXTURES_PATH + "/lamp/normal.png"))
            .setEmissiveMap(rm.get<Image>(TEXTURES_PATH + "white_pixel.png"))//"/lamp/emissive.png"))
            .setEmissiveColor(glm::vec4{ 1.0f, 1.0f, 1.0f, 6.0f })
            .build();
        rm.add(lampMaterial, "lamp_material");

        auto lampMesh = rm.get<Mesh>(MODELS_PATH + "lamp.obj");

        Entity entity = getScene().createEntity("lamp")
            .add<TransformComponent>(glm::vec3{ 0.6f, 1.0f, 0.6f }, glm::vec3{ 2.4f, 2.8f, 2.4f }, glm::vec3{ glm::pi<float>(), glm::pi<float>() / 4, 0.0f })
            .add<MeshComponent>(lampMesh)
            .add<MaterialComponent>(MaterialComponent::Builder()
                .setMaterial(lampMaterial).build());
    }

    void createRoofLight() {
        auto& rm = getResourceManager();

        ImageInfo albedoInfo{};
        albedoInfo.format = RGBA8_SRGB;

        auto roofLightMaterial = Material::Builder()
            .setEmissiveMap(rm.get<Image>(TEXTURES_PATH + "white_pixel.png"))
            .setEmissiveColor(glm::vec4{ 1.0f, 1.0f, 1.0f, 12.0f})
            .build();
        rm.add(roofLightMaterial, "roof_light_material");

        auto roofLightMesh = rm.get<Mesh>(MODELS_PATH + "cube.obj");

        Entity entity = getScene().createEntity("lamp")
            .add<TransformComponent>(glm::vec3{ 0.0f, -0.995f, 0.0f }, glm::vec3{ 0.25f, 0.01f, 0.25f }, glm::vec3{ glm::pi<float>(), 0.0, 0.0})
            .add<MeshComponent>(roofLightMesh)
            .add<MaterialComponent>(MaterialComponent::Builder()
                .setMaterial(roofLightMaterial).build());
    }

    void createPencilAndPen() {
        auto& rm = getResourceManager();

        ImageInfo albedoInfo{};
        albedoInfo.format = RGBA8_SRGB;

        auto pencilMaterial = Material::Builder()
            .setAlbedoMap(rm.get<Image>(TEXTURES_PATH + "/pencil/albedo.png", &albedoInfo))
            .setRoughnessMap(rm.get<Image>(TEXTURES_PATH + "/pencil/roughness.png"))
			.setMetallicMap(rm.get<Image>(TEXTURES_PATH + "/pencil/metallic.png"))
            .setNormalMap(rm.get<Image>(TEXTURES_PATH + "/pencil/normal.png"))
            .build();
        rm.add(pencilMaterial, "pencil_material");

        auto pencilMesh = rm.get<Mesh>(MODELS_PATH + "pencil.obj");

        Entity entity = getScene().createEntity("pencil")
            .add<TransformComponent>(glm::vec3{ 0.65f, 0.985f, -0.1f }, glm::vec3{ 0.1f, 0.1f, 0.1f }, glm::vec3{ 0.0f, -glm::pi<float>() / 10, 0.0f })
            .add<MeshComponent>(pencilMesh)
            .add<MaterialComponent>(MaterialComponent::Builder()
                .setMaterial(pencilMaterial).build());

        entity = getScene().createEntity("pencil2")
            .add<TransformComponent>(glm::vec3{ 0.55f, 0.985f, 0.0f }, glm::vec3{ 0.1f, 0.1f, 0.1f }, glm::vec3{ 0.0f, -glm::pi<float>() / 12, 0.0f })
            .add<MeshComponent>(pencilMesh)
            .add<MaterialComponent>(MaterialComponent::Builder()
                .setMaterial(pencilMaterial).build());
        
    }

    void createLights() {
        //entity = createPointLightEntity(0.25f, 0.02f, glm::vec3{1.f, 1.f, 1.f});
        //entity.get<TransformComponent>().translation = glm::vec3{0.0f, 0.0f, 0.0f};

        // Three rotating lights (white, green, blue)
        Entity entity = createPointLightEntity(1.0f, 0.025f, glm::vec3{ 1.f, 1.f, 1.f });
        entity.get<TransformComponent>().translation = glm::vec3{ 1.0f / (float) sqrt(3), 0.5f, 0.2f };
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
        createTeapotAndVases(5);
        //createRubikCube();
        //createLamp();
		createRoofLight();
        createPencilAndPen();
        createLights();

        auto& rm = getResourceManager();

        ImageInfo albedoInfo{};
        albedoInfo.format = RGBA8_SRGB;

        auto bunny = rm.get<Mesh>(MODELS_PATH + "bunny/bunny.obj");
        /*auto bunnyMaterial = Material::Builder()
            .setAlbedoMap(rm.get<Image>(MODELS_PATH + "bunny/terracotta.jpg", &albedoInfo))
            //.setAlbedoMap(rm.get<Image>(TEXTURES_PATH + "granite/albedo.png", &albedoInfo))
            .setRoughnessMap(rm.get<Image>(TEXTURES_PATH + "granite/roughness.png"))
            .setMetallicMap(rm.get<Image>(TEXTURES_PATH + "granite/metallic.png"))
            .setNormalMap(rm.get<Image>(TEXTURES_PATH + "granite/normal.png"))
            .setAmbientOcclusionMap(rm.get<Image>(TEXTURES_PATH + "granite/ao.png"))
            .build();*/
        auto bunnyMaterial = Material::Builder()
            .setRoughnessMap(rm.get<Image>(TEXTURES_PATH + "/gold/roughness.png"))
            .setMetallicMap(rm.get<Image>(TEXTURES_PATH + "/gold/metallic.png"))
            .setNormalMap(rm.get<Image>(TEXTURES_PATH + "/gold/normal.png"))
            .build();
		rm.add(bunnyMaterial, "bunny_material");

        Entity entity = getScene().createEntity("Bunny")
            .add<TransformComponent>(glm::vec3{ 0.0f, 0.99f, 0.0f }, glm::vec3{ 2.5f, 2.5f, 2.5f }, glm::vec3{ glm::pi<float>(), 0.0f, 0.0f })
            .add<MeshComponent>(bunny)
            .add<MaterialComponent>(MaterialComponent::Builder()
                .setMaterial(bunnyMaterial)
                .setTint(glm::vec3(1.0, 0.812, 0.408))
                //.setTilingFactor(5.0f)
                .build());
    }

    
};

PXTEngine::Application* PXTEngine::initApplication() {
    return new App();
}