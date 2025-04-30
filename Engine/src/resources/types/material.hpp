#pragma once

#include "core/memory.hpp"
#include "resources/resource.hpp"
#include "resources/types/image.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/glm.hpp>

namespace PXTEngine {

    class Material : public Resource {
    public:
		class Builder {
		public:
			Builder() = default;

			Builder& setAlbedoColor(const glm::vec4& color) {
				m_albedoColor = color;
				return *this;
			}

			Builder& setAlbedoMap(Shared<Image> map) {
				m_albedoMap = map;
				return *this;
			}

			Builder& setMetallicRoughnessMap(Shared<Image> map) {
				m_metallicRoughnessMap = map;
				return *this;
			}

			Builder& setNormalMap(Shared<Image> map) {
				m_normalMap = map;
				return *this;
			}

			Builder& setAmbientOcclusionMap(Shared<Image> map) {
				m_ambientOcclusionMap = map;
				return *this;
			}

			Builder& setEmissiveFactor(const glm::vec3& factor) {
				m_emissiveFactor = factor;
				return *this;
			}

			Builder& setEmissiveMap(Shared<Image> map) {
				m_emissiveMap = map;
				return *this;
			}

			Shared<Material> build() {
				return createShared<Material>(m_albedoColor, m_albedoMap, m_normalMap, m_metallicRoughnessMap,m_ambientOcclusionMap, m_emissiveFactor, m_emissiveMap);
			}

		protected:
			glm::vec4 m_albedoColor{ 1.0f };
			Shared<Image> m_albedoMap{ nullptr };
			Shared<Image> m_normalMap{ nullptr };
			Shared<Image> m_metallicRoughnessMap{ nullptr };
			Shared<Image> m_ambientOcclusionMap{ nullptr };
			glm::vec3 m_emissiveFactor{ 0.0f, 0.0f, 0.0f };
			Shared<Image> m_emissiveMap{ nullptr };
		};

		Material(
			const glm::vec4& albedoColor,
			const Shared<Image>& albedoMap,
			const Shared<Image>& normalMap,
			const Shared<Image>& metallicRoughnessMap,
			const Shared<Image>& ambientOcclusionMap,
			const glm::vec3& emissiveFactor,
			const Shared<Image>& emissiveMap)
			: m_albedoColor(albedoColor),
			m_albedoMap(albedoMap),
			m_normalMap(normalMap),
			m_metallicRoughnessMap(metallicRoughnessMap),
			m_ambientOcclusionMap(ambientOcclusionMap),
			m_emissiveFactor(emissiveFactor),
			m_emissiveMap(emissiveMap) {}

		static Type getStaticType() { return Type::Material; }

		Type getType() const override {
			return Type::Material;
		}

		const glm::vec4& getAlbedoColor() const {
			return m_albedoColor;
		}

		Shared<Image> getAlbedoMap() const {
			return m_albedoMap;
		}

		Shared<Image> getMetallicRoughnessMap() const {
			return m_metallicRoughnessMap;
		}

		Shared<Image> getNormalMap() const {
			return m_normalMap;
		}

        Shared<Image> getAmbientOcclusionMap() const {
			return m_ambientOcclusionMap;
		}

        const glm::vec3& getEmissiveFactor() const {
			return m_emissiveFactor;
		}

        Shared<Image> getEmissiveMap() const {
			return m_emissiveMap;
		}

	protected:
		glm::vec4 m_albedoColor{ 1.0f, 1.0f, 1.0f, 1.0f };
		Shared<Image> m_albedoMap{ nullptr };

		Shared<Image> m_normalMap{ nullptr };

		Shared<Image> m_metallicRoughnessMap{ nullptr };

		Shared<Image> m_ambientOcclusionMap{ nullptr };

		glm::vec3 m_emissiveFactor{ 0.0f, 0.0f, 0.0f };
		Shared<Image> m_emissiveMap{ nullptr };
    };
}