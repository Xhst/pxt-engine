#pragma once

#include "scene/camera.hpp"
#include "scene/scene.hpp"

#include <vulkan/vulkan.h>

namespace PXTEngine {

    #define MAX_LIGHTS 10

    struct PointLight {
        glm::vec4 position{};
        glm::vec4 color{}; // w is intensity
    };

    struct GlobalUbo {
        glm::mat4 projection{1.f};
        glm::mat4 view{1.f};
        glm::mat4 inverseView{1.f};
        glm::vec4 ambientLightColor{0.67f, 0.85f, 0.9f, .02f};
        PointLight pointLights[MAX_LIGHTS];
        int numLights;
        uint32_t frameCount;
        uint32_t ptAccumulationCount;
    };

    struct FrameInfo {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        Camera& camera;
        VkDescriptorSet globalDescriptorSet;
        Scene& scene;
    };
}