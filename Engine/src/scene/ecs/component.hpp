
#pragma once

#include "application.hpp"
#include "core/uuid.hpp"
#include "core/memory.hpp"
#include "core/constants.hpp"
#include "resources/types/mesh.hpp"
#include "resources/types/material.hpp"
#include "scene/camera.hpp"

#include <glm/glm.hpp>

namespace PXTEngine
{
    struct IDComponent {
        UUID uuid;

		IDComponent(UUID uuid) : uuid(uuid) {}
        IDComponent(const IDComponent&) = default;

        operator UUID&() { return uuid; }
        operator const UUID&() const { return uuid; }
    };

    struct NameComponent {
        std::string name;

        NameComponent() = default;
        NameComponent(const NameComponent&) = default;
        NameComponent(const std::string& name) : name(name) {}

        operator std::string&() { return name; }
        operator const std::string&() const { return name; }
    };

    struct ColorComponent {
        glm::vec3 color;

        ColorComponent() = default;
        ColorComponent(const ColorComponent&) = default;
        ColorComponent(const glm::vec3& color) : color(color) {}

        operator glm::vec3&() { return color; }
        operator const glm::vec3&() const { return color; }
    };

    struct MaterialComponent {
		Shared<Material> material;
		float tilingFactor = 1.0f;
        glm::vec3 tint{ 1.0f };

		MaterialComponent() {
			auto& rm = Application::get().getResourceManager();

			material = rm.get<Material>(DEFAULT_MATERIAL);
		}

		MaterialComponent(const MaterialComponent&) = default;
		MaterialComponent(const Shared<Material>& material, float tilingFactor, const glm::vec3& tint) : material(material), tilingFactor(tilingFactor), tint(tint) {}

		struct Builder {
			Shared<Material> material;
			float tilingFactor = 1.0f;
			glm::vec3 tint{ 1.0f };

			Builder& setMaterial(const Shared<Material>& material) {
				this->material = material;
				return *this;
			}

			Builder& setTilingFactor(float tilingFactor) {
				this->tilingFactor = tilingFactor;
				return *this;
			}

			Builder& setTint(const glm::vec3& tint) {
				this->tint = tint;
				return *this;
			}

			MaterialComponent build() {
				return MaterialComponent(material, tilingFactor, tint);
			}
		};
    };

    struct Transform2dComponent {
        glm::vec2 translation{};
        glm::vec2 scale{1.f, 1.f};
        float rotation;

        glm::mat2 mat2() {
            const float sin = glm::sin(rotation);
            const float cos = glm::cos(rotation);

            glm::mat2 rotationMatrix{{cos, sin}, {-sin, cos}};
            glm::mat2 scaleMatrix{{scale.x, 0.f}, {0.f, scale.y}};
            
            return rotationMatrix * scaleMatrix;
        }

        Transform2dComponent() = default;
        Transform2dComponent(const Transform2dComponent&) = default;
        Transform2dComponent(const glm::vec2& translation) 
            : translation(translation), rotation(0.0f) {}

        Transform2dComponent(const glm::vec2& translation, const glm::vec2& scale) 
            : translation(translation), scale(scale), rotation(0.0f) {}

        Transform2dComponent(const glm::vec2& translation, const glm::vec2& scale, const float rotation) 
            : translation(translation), scale(scale), rotation(rotation) {}
    
        operator glm::mat2() { return mat2(); }
    };

    struct TransformComponent {
        glm::vec3 translation{};
        glm::vec3 scale{1.f, 1.f, 1.f};
        glm::vec3 rotation{};

        /**
         * @brief Transforms the entity's position, scale, and rotation into a 4x4 matrix
         * 
         * Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
         * Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
         * 
         * To view the rotation as extrinsic, just read the operations from right to left
         * Otherwise, to view the rotation as intrinsic, read the operations from left to right
         * 
         * - Extrinsic: Z(world) -> X(world) -> Y(world)
         * 
         * - Intrinsic: Y(local) -> X(local) -> Z(local)
         * 
         * @note https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
         * 
         * @return glm::mat4
         */
        glm::mat4 mat4() {
            const float c3 = glm::cos(rotation.z);
            const float s3 = glm::sin(rotation.z);
            const float c2 = glm::cos(rotation.x);
            const float s2 = glm::sin(rotation.x);
            const float c1 = glm::cos(rotation.y);
            const float s1 = glm::sin(rotation.y);
            return glm::mat4{
                {
                    scale.x * (c1 * c3 + s1 * s2 * s3),
                    scale.x * (c2 * s3),
                    scale.x * (c1 * s2 * s3 - c3 * s1),
                    0.0f,
                },
                {
                    scale.y * (c3 * s1 * s2 - c1 * s3),
                    scale.y * (c2 * c3),
                    scale.y * (c1 * c3 * s2 + s1 * s3),
                    0.0f,
                },
                {
                    scale.z * (c2 * s1),
                    scale.z * (-s2),
                    scale.z * (c1 * c2),
                    0.0f,
                },
                {translation.x, translation.y, translation.z, 1.0f}};
        }

        glm::mat3 normalMatrix() {
            const float c3 = glm::cos(rotation.z);
            const float s3 = glm::sin(rotation.z);
            const float c2 = glm::cos(rotation.x);
            const float s2 = glm::sin(rotation.x);
            const float c1 = glm::cos(rotation.y);
            const float s1 = glm::sin(rotation.y);
            const glm::vec3 inverseScale = 1.0f / scale;

            return glm::mat3{
                {
                    inverseScale.x* (c1 * c3 + s1 * s2 * s3),
                    inverseScale.x * (c2 * s3),
                    inverseScale.x * (c1 * s2 * s3 - c3 * s1),
                },
                {
                    inverseScale.y * (c3 * s1 * s2 - c1 * s3),
                    inverseScale.y * (c2 * c3),
                    inverseScale.y * (c1 * c3 * s2 + s1 * s3),
                },
                {
                    inverseScale.z * (c2 * s1),
                    inverseScale.z * (-s2),
                    inverseScale.z * (c1 * c2),
                },
            };
        }

        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;
        TransformComponent(const glm::vec3& translation) 
            : translation(translation) {}

        TransformComponent(const glm::vec3& translation, const glm::vec3& scale) 
            : translation(translation), scale(scale) {}

        TransformComponent(const glm::vec3& translation, const glm::vec3& scale, const glm::vec3& rotation) 
            : translation(translation), scale(scale), rotation(rotation) {}
    
        operator glm::mat4() { return mat4(); }
    };

    struct MeshComponent {
        Shared<Mesh> mesh;

        MeshComponent() = default;
        MeshComponent(const MeshComponent&) = default;
        MeshComponent(const Shared<Mesh>& mesh) : mesh(mesh) {}
    };

    class Script;

    struct ScriptComponent {
        Script* script = nullptr;

        Script*(*create)();
		void (*destroy)(ScriptComponent*);

		template<typename T>
		void bind()
		{
			create  = []() { 
                return static_cast<Script*>(new T()); 
            };

			destroy = [](ScriptComponent* s) { 
                delete s->script; 
                s->script = nullptr; 
            };
		}

    };

    struct CameraComponent {
        Camera camera;

        bool isMainCamera = true;

        CameraComponent() {
            camera = Camera{};
        }
        CameraComponent(const CameraComponent&) = default;
    };

    struct PointLightComponent {
        float lightIntensity = 1.0f;

        PointLightComponent() = default;
        PointLightComponent(const PointLightComponent&) = default;
        PointLightComponent(const float intensity) : lightIntensity(intensity) {}
    };
}