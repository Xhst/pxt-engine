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

	/**
	 * @class Material
	 *
	 * @brief Represents a material resource used for rendering.
	 * This class encapsulates various properties of a material, including color, textures,
	 * and other parameters.
	 */
    class Material : public Resource {
    public:
        class Builder {
        public:
            Builder& setAlbedoColor(const glm::vec4& color);
            Builder& setAlbedoMap(Shared<Image> map);
            Builder& setMetallicMap(Shared<Image> map);
            Builder& setRoughnessMap(Shared<Image> map);
            Builder& setNormalMap(Shared<Image> map);
            Builder& setAmbientOcclusionMap(Shared<Image> map);
            Builder& setEmissiveColor(const glm::vec4& color);
            Builder& setEmissiveMap(Shared<Image> map);
            Shared<Material> build();

        protected:
            glm::vec4 m_albedoColor{ 1.0f };
            Shared<Image> m_albedoMap{ nullptr };
            Shared<Image> m_normalMap{ nullptr };
            Shared<Image> m_metallicMap{ nullptr };
            Shared<Image> m_roughnessMap{ nullptr };
            Shared<Image> m_ambientOcclusionMap{ nullptr };
            glm::vec4 m_emissiveColor{ 0.0f };
            Shared<Image> m_emissiveMap{ nullptr };
        };

        Material(
            const glm::vec4& albedoColor,
            const Shared<Image>& albedoMap,
            const Shared<Image>& normalMap,
            const Shared<Image>& metallicMap,
            const Shared<Image>& roughnessMap,
            const Shared<Image>& ambientOcclusionMap,
            const glm::vec4& emissiveColor,
            const Shared<Image>& emissiveMap
        );

        static Type getStaticType();
        Type getType() const override;

        const glm::vec4& getAlbedoColor() const;
        Shared<Image> getAlbedoMap() const;
        Shared<Image> getMetallicMap() const;
        Shared<Image> getRoughnessMap() const;
        Shared<Image> getNormalMap() const;
        Shared<Image> getAmbientOcclusionMap() const;
        const glm::vec4& getEmissiveColor() const;
        Shared<Image> getEmissiveMap() const;

        bool isEmissive();

    protected:
        glm::vec4 m_albedoColor{ 1.0f };
        Shared<Image> m_albedoMap{ nullptr };
        Shared<Image> m_normalMap{ nullptr };
        Shared<Image> m_metallicMap{ nullptr };
        Shared<Image> m_roughnessMap{ nullptr };
        Shared<Image> m_ambientOcclusionMap{ nullptr };
        glm::vec4 m_emissiveColor{ 0.0f };
        Shared<Image> m_emissiveMap{ nullptr };
    };
}