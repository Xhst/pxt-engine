#pragma once

#include "scene/camera.hpp"
#include "scene/scene.hpp"

#include <vulkan/vulkan.h>

namespace CGEngine {
    struct FrameInfo {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        Camera& camera;
        VkDescriptorSet globalDescriptorSet;
        Scene& scene;
    };
}