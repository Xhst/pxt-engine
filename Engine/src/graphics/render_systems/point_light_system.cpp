#include "graphics/render_systems/point_light_system.hpp"
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

    struct PointLightPushConstants {
        glm::vec4 position{};
        glm::vec4 color{};
        float radius;
    };

    PointLightSystem::PointLightSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : m_device(device) {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    PointLightSystem::~PointLightSystem() {
        vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
    }

    void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PointLightPushConstants);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};


        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void PointLightSystem::createPipeline(VkRenderPass renderPass) {
        assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipelineLayout");

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);

        // clear model information when using point lights system
        pipelineConfig.bindingDescriptions.clear();
        pipelineConfig.attributeDescriptions.clear();

        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_pipelineLayout;

        m_pipeline = createUnique<Pipeline>(
            m_device,
            "../build/shaders/point_light.vert.spv",
            "../build/shaders/point_light.frag.spv",
            pipelineConfig
        );
    }

    void PointLightSystem::update(FrameInfo& frameInfo, GlobalUbo& ubo) {
        int lightIndex = 0;

        auto view = frameInfo.scene.getEntitiesWith<PointLightComponent, ColorComponent, TransformComponent>();
        for (auto entity : view) {

            const auto&[light, color, transform] = view.get<PointLightComponent, ColorComponent, TransformComponent>(entity);

            //update lights in the ubo
            ubo.pointLights[lightIndex].position = glm::vec4(transform.translation, 1.f);
            ubo.pointLights[lightIndex].color = glm::vec4((glm::vec3) color, light.lightIntensity);

            lightIndex += 1;
        }

        ubo.numLights = lightIndex;
    }

    void PointLightSystem::render(FrameInfo& frameInfo) {
        m_pipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipelineLayout,
            0,
            1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr
        );

        auto view = frameInfo.scene.getEntitiesWith<PointLightComponent, ColorComponent, TransformComponent>();
        for (auto entity : view) {

            const auto&[light, color, transform] = view.get<PointLightComponent, ColorComponent, TransformComponent>(entity);

            PointLightPushConstants push{};
            push.position = glm::vec4(transform.translation, 1.f);
            push.color = glm::vec4((glm::vec3) color, light.lightIntensity);
            push.radius = transform.scale.x;

            vkCmdPushConstants(
                frameInfo.commandBuffer,
                m_pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(PointLightPushConstants),
                &push
            );
            
            vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
        }
    }
}