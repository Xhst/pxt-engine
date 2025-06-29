#include "scene/ecs/component.hpp"

#include "application.hpp"  

namespace PXTEngine
{
	// --- MaterialComponent ---
	MaterialComponent::MaterialComponent()
		: tilingFactor(1.0f), tint(1.0f)
	{
		auto& rm = Application::get().getResourceManager();
		material = rm.get<Material>(DEFAULT_MATERIAL);
	}

	// --- Transform2dComponent ---
	glm::mat2 Transform2dComponent::mat2() {
		const float sin = glm::sin(rotation);
		const float cos = glm::cos(rotation);

		glm::mat2 rotationMatrix(cos, sin, -sin, cos);
		glm::mat2 scaleMatrix(scale.x, 0.f, 0.f, scale.y);

		return rotationMatrix * scaleMatrix;
	}

	// --- TransformComponent ---
	glm::mat4 TransformComponent::mat4() {
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
			{translation.x, translation.y, translation.z, 1.0f} };
	}

	glm::mat3 TransformComponent::normalMatrix() {
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		const glm::vec3 inverseScale = 1.0f / scale;

		return glm::mat3{
			{
				inverseScale.x * (c1 * c3 + s1 * s2 * s3),
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

	// --- CameraComponent ---
	CameraComponent::CameraComponent()
		: isMainCamera(true)
	{
		
		camera = Camera{};
	}

}