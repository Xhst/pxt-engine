#pragma once

#include "core/pch.hpp"
#include "core/uuid.hpp"               
#include "resources/types/mesh.hpp"
#include "resources/types/material.hpp" 
#include "scene/camera.hpp"       
           

namespace PXTEngine
{
	struct IDComponent {
		UUID uuid;

		IDComponent(UUID uuid) : uuid(uuid) {}
		IDComponent(const IDComponent&) = default;

		// Conversion operators
		operator UUID& () { return uuid; }
		operator const UUID& () const { return uuid; }
	};

	struct NameComponent {
		std::string name;

		NameComponent() = default;
		NameComponent(const NameComponent&) = default;
		
		NameComponent(const std::string& name) : name(name) {}

		// Conversion operators
		operator std::string& () { return name; }
		operator const std::string& () const { return name; }
	};

	struct ColorComponent {
		glm::vec3 color;

		ColorComponent() = default;
		ColorComponent(const ColorComponent&) = default;
		
		ColorComponent(const glm::vec3& color) : color(color) {}

		// Conversion operators
		operator glm::vec3& () { return color; }
		operator const glm::vec3& () const { return color; }
	};

	struct MaterialComponent {
		Shared<Material> material;
		float tilingFactor = 1.0f;
		glm::vec3 tint{ 1.0f };

		MaterialComponent();

		MaterialComponent(const MaterialComponent&) = default;
		
		MaterialComponent(const Shared<Material>& material, float tilingFactor, const glm::vec3& tint)
			: material(material), tilingFactor(tilingFactor), tint(tint) {
		}

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
		glm::vec2 scale{ 1.f, 1.f };
		float rotation = 0.0f;

		glm::mat2 mat2();

		Transform2dComponent() = default;
		Transform2dComponent(const Transform2dComponent&) = default;
		
		Transform2dComponent(const glm::vec2& translation)
			: translation(translation) {
		}

		Transform2dComponent(const glm::vec2& translation, const glm::vec2& scale)
			: translation(translation), scale(scale) {
		}

		Transform2dComponent(const glm::vec2& translation, const glm::vec2& scale, const float rotation)
			: translation(translation), scale(scale), rotation(rotation) {
		}

		operator glm::mat2() { return mat2(); }
	};

	struct TransformComponent {
		glm::vec3 translation{};
		glm::vec3 scale{ 1.f, 1.f, 1.f };
		glm::vec3 rotation{};

		/**
		 * @brief Transforms the entity's position, scale, and rotation into a 4x4 matrix
		 *
		 * Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
		 * Rotations correspond to Tait-Bryan angles of Y(1), X(2), Z(3)
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
		glm::mat4 mat4();
		glm::mat3 normalMatrix();

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		
		TransformComponent(const glm::vec3& translation)
			: translation(translation) {
		}

		TransformComponent(const glm::vec3& translation, const glm::vec3& scale)
			: translation(translation), scale(scale) {
		}

		TransformComponent(const glm::vec3& translation, const glm::vec3& scale, const glm::vec3& rotation)
			: translation(translation), scale(scale), rotation(rotation) {
		}

		// Conversion operator calling the mat4 function
		operator glm::mat4() { return mat4(); }
	};

	struct MeshComponent {
		Shared<Mesh> mesh;

		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;

		MeshComponent(const Shared<Mesh>& mesh) : mesh(mesh) {}
	};

	class Script; // Forward declaration of Script class			
	struct ScriptComponent {
		Script* script = nullptr;

		// Function pointers for creating and destroying scripts
		Script* (*create)() = nullptr;
		void (*destroy)(ScriptComponent*) = nullptr;

		template<typename T>
		void bind()
		{
			create = []() {
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

		CameraComponent();

		CameraComponent(const CameraComponent&) = default;
	};

	struct PointLightComponent {
		float lightIntensity = 1.0f;

		PointLightComponent() = default;
		PointLightComponent(const PointLightComponent&) = default;
		
		PointLightComponent(const float intensity) : lightIntensity(intensity) {}
	};
}