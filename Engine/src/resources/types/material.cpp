#include "resources/types/material.hpp"

#include "resources/resource_manager.hpp"

namespace PXTEngine {
    Material::Material(
        const glm::vec4& albedoColor,
        const Shared<Image>& albedoMap,
        const Shared<Image>& normalMap,
        const Shared<Image>& metallicMap,
        const Shared<Image>& roughnessMap,
        const Shared<Image>& ambientOcclusionMap,
        const glm::vec3& emissiveFactor,
        const Shared<Image>& emissiveMap)
        : m_albedoColor(albedoColor),
        m_albedoMap(albedoMap),
        m_normalMap(normalMap),
        m_metallicMap(metallicMap),
        m_roughnessMap(roughnessMap),
        m_ambientOcclusionMap(ambientOcclusionMap),
        m_emissiveFactor(emissiveFactor),
        m_emissiveMap(emissiveMap) {}

    Material::Type Material::getStaticType() {
        return Type::Material;
    }

    Material::Type Material::getType() const {
        return Type::Material;
    }

    const glm::vec4& Material::getAlbedoColor() const { return m_albedoColor; }
    Shared<Image> Material::getAlbedoMap() const { return m_albedoMap; }
    Shared<Image> Material::getMetallicMap() const { return m_metallicMap; }
    Shared<Image> Material::getRoughnessMap() const { return m_roughnessMap; }
    Shared<Image> Material::getNormalMap() const { return m_normalMap; }
    Shared<Image> Material::getAmbientOcclusionMap() const { return m_ambientOcclusionMap; }
    const glm::vec3& Material::getEmissiveFactor() const { return m_emissiveFactor; }
    Shared<Image> Material::getEmissiveMap() const { return m_emissiveMap; }

    // -------- Builder Implementation --------

    Material::Builder& Material::Builder::setAlbedoColor(const glm::vec4& color) {
        m_albedoColor = color;
        return *this;
    }

    Material::Builder& Material::Builder::setAlbedoMap(Shared<Image> map) {
        m_albedoMap = map;
        return *this;
    }

    Material::Builder& Material::Builder::setMetallicMap(Shared<Image> map) {
        m_metallicMap = map;
        return *this;
    }

    Material::Builder& Material::Builder::setRoughnessMap(Shared<Image> map) {
        m_roughnessMap = map;
        return *this;
    }

    Material::Builder& Material::Builder::setNormalMap(Shared<Image> map) {
        m_normalMap = map;
        return *this;
    }

    Material::Builder& Material::Builder::setAmbientOcclusionMap(Shared<Image> map) {
        m_ambientOcclusionMap = map;
        return *this;
    }

    Material::Builder& Material::Builder::setEmissiveFactor(const glm::vec3& factor) {
        m_emissiveFactor = factor;
        return *this;
    }

    Material::Builder& Material::Builder::setEmissiveMap(Shared<Image> map) {
        m_emissiveMap = map;
        return *this;
    }

    Shared<Material> Material::Builder::build() {
        if (!m_albedoMap) m_albedoMap = ResourceManager::defaultMaterial->getAlbedoMap();
        if (!m_normalMap) m_normalMap = ResourceManager::defaultMaterial->getNormalMap();
        if (!m_metallicMap) m_metallicMap = ResourceManager::defaultMaterial->getMetallicMap();
        if (!m_roughnessMap) m_roughnessMap = ResourceManager::defaultMaterial->getRoughnessMap();
        if (!m_ambientOcclusionMap) m_ambientOcclusionMap = ResourceManager::defaultMaterial->getAmbientOcclusionMap();
        if (!m_emissiveMap) m_emissiveMap = ResourceManager::defaultMaterial->getEmissiveMap();

        return createShared<Material>(
            m_albedoColor,
            m_albedoMap,
            m_normalMap,
            m_metallicMap,
            m_roughnessMap,
            m_ambientOcclusionMap,
            m_emissiveFactor,
            m_emissiveMap
        );
    }
}