#pragma once

#include "scene/camera.hpp"

#include <vulkan/vulkan.h>

namespace CGEngine {
    struct FrameInfo {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        Camera& camera;
        VkDescriptorSet globalDescriptorSet;
    };
}