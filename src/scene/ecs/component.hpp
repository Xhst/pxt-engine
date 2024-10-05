
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
        glm::vec4 color;

        ColorComponent() = default;
        ColorComponent(const ColorComponent&) = default;
        ColorComponent(const glm::vec4& color) : color(color) {}
        ColorComponent(const glm::vec3& color) : color(glm::vec4{color, 1.f}) {}

        operator glm::vec4&() { return color; }
        operator const glm::vec4&() const { return color; }
        operator glm::vec3() const { return glm::vec3(color); }
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
            : translation(translation) {}

        Transform2dComponent(const glm::vec2& translation, const glm::vec2& scale) 
            : translation(translation), scale(scale) {}

        Transform2dComponent(const glm::vec2& translation, const glm::vec2& scale, float rotation) 
            : translation(translation), scale(scale), rotation(rotation) {}
    
        operator glm::mat2() { return mat2(); }
    };

    struct ModelComponent {
        Shared<Model> model;

        ModelComponent() = default;
        ModelComponent(const ModelComponent&) = default;
        ModelComponent(const Shared<Model>& model) : model(model) {}

        operator Shared<Model>&() { return model; }
        operator const Shared<Model>&() const { return model; }
    };
}