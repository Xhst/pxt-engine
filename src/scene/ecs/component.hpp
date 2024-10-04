
#pragma once

#include "core/uuid.hpp"
#include "core/memory.hpp"
#include "graphics/model.hpp"

#include <glm/glm.hpp>

namespace CGEngine
{
    struct IDComponent {
        UUID uuid;

        IDComponent() = default;
        IDComponent(const IDComponent&) = default;
    };

    struct NameComponent {
        std::string name;

        NameComponent() = default;
        NameComponent(const NameComponent&) = default;
        NameComponent(const std::string& name) : name(name) {}
    };

    struct ColorComponent {
        glm::vec4 color;

        ColorComponent() = default;
        ColorComponent(const ColorComponent&) = default;
        ColorComponent(const glm::vec4& color) : color(color) {}
        ColorComponent(const glm::vec3& color) : color(glm::vec4{color, 1.f}) {}
    };

    struct Transform2dComponent {
        glm::vec2 translation{};
        glm::vec2 scale{1.f, 1.f};
        float rotation;

        glm::mat2 mat2 () {
            const float sin = glm::sin(rotation);
            const float cos = glm::cos(rotation);
            glm::mat2 rotationMatrix{{cos, sin}, {-sin, cos}};

            glm::mat2 scaleMatrix{{scale.x, 0.f}, {0.f, scale.y}};
            return rotationMatrix * scaleMatrix;
        }

        Transform2dComponent() = default;
        Transform2dComponent(const Transform2dComponent&) = default;
        Transform2dComponent(const glm::vec2& translation) : translation(translation) {}
        Transform2dComponent(const glm::vec2& translation, const glm::vec2& scale) : translation(translation), scale(scale) {}
        Transform2dComponent(const glm::vec2& translation, const glm::vec2& scale, float rotation) : translation(translation), scale(scale), rotation(rotation) {}
    };

    struct ModelComponent {
        Shared<Model> model;

        ModelComponent() = default;
        ModelComponent(const ModelComponent&) = default;
        ModelComponent(const Shared<Model>& model) : model(model) {}
    };
}