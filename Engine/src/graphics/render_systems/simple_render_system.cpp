#include "graphics/render_systems/simple_render_system.hpp"
#include "core/memory.hpp"
#include "scene/ecs/entity.hpp"

#include <iostream>
#include <array>
#include <stdexcept>
#include <cassert>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace CGEngine {

    struct SimplePushConstantData {
        glm::mat4 transform{1.f};
        glm::mat4 normalMatrix{1.f};
    };

    SimpleRenderSystem::SimpleRenderSystem(Device& device, VkRenderPass renderPass) : m_device(device) {
        createPipelineLayout();
        createPipeline(renderPass);
    }

    SimpleRenderSystem::~SimpleRenderSystem() {
        vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
    }

    void SimpleRenderSystem::createPipelineLayout() {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
        assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipelineLayout");

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_pipelineLayout;

        m_pipeline = createUnique<Pipeline>(
            m_device,
            "../build/shaders/simple_shader.vert.spv",
            "../build/shaders/simple_shader.frag.spv",
            pipelineConfig
        );
    }

    void SimpleRenderSystem::renderScene(FrameInfo& frameInfo, Scene& scene) {
        m_pipeline->bind(frameInfo.commandBuffer);

        auto projectionView = frameInfo.camera.getProjectionMatrix() * frameInfo.camera.getViewMatrix();

        auto view = scene.getEntitiesWith<TransformComponent, ColorComponent, ModelComponent>();
        for (auto entity : view) {

            const auto&[transform, color, model] = view.get<TransformComponent, ColorComponent, ModelComponent>(entity);

            SimplePushConstantData push{};
            auto modelMatrix = transform.mat4();
            push.transform = projectionView * modelMatrix;
            push.normalMatrix = transform.normalMatrix();

            vkCmdPushConstants(
                frameInfo.commandBuffer,
                m_pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(SimplePushConstantData),
                &push);

            auto modelPtr = model.model;
            
            modelPtr->bind(frameInfo.commandBuffer);
            modelPtr->draw(frameInfo.commandBuffer);

        }
    }
}