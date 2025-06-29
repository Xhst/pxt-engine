#include "graphics/render_systems/point_light_system.hpp"

#include "scene/ecs/entity.hpp"

namespace PXTEngine {

    struct PointLightPushConstants {
        glm::vec4 position{};
        glm::vec4 color{};
        float radius;
    };

    PointLightSystem::PointLightSystem(Context& context, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : m_context(context) {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    PointLightSystem::~PointLightSystem() {
        vkDestroyPipelineLayout(m_context.getDevice(), m_pipelineLayout, nullptr);
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

        if (vkCreatePipelineLayout(m_context.getDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }



    void PointLightSystem::createPipeline(VkRenderPass renderPass, bool useCompiledSpirvFiles) {
        PXT_ASSERT(m_pipelineLayout != nullptr, "Cannot create pipeline before pipelineLayout");

        RasterizationPipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        Pipeline::enableAlphaBlending(pipelineConfig);

        // clear model information when using point lights system
        pipelineConfig.bindingDescriptions.clear();
        pipelineConfig.attributeDescriptions.clear();

        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_pipelineLayout;

        const std::string baseShaderPath = useCompiledSpirvFiles ? SPV_SHADERS_PATH : SHADERS_PATH;
        const std::string filenameSuffix = useCompiledSpirvFiles ? ".spv" : "";

        std::vector<std::string> shaderFilePaths;
        for (const auto& filePath : m_shaderFilePaths) {
            shaderFilePaths.push_back(baseShaderPath + filePath + filenameSuffix);
        };

		m_pipeline = createUnique<Pipeline>(
			m_context,
            shaderFilePaths,
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
        // sort lights by distance to camera
        //TODO: WE SHOULD DO THIS FOR EVERY TRANSPARENT OBJECT or use order independent transparency
        std::map<float, entt::entity> sorted;

        auto view = frameInfo.scene.getEntitiesWith<PointLightComponent, ColorComponent, TransformComponent>();
        for (auto entity : view) {

            const auto&[light, color, transform] = view.get<PointLightComponent, ColorComponent, TransformComponent>(entity);

            glm::vec3 lightPos = transform.translation;
            glm::vec3 cameraPos = frameInfo.camera.getPosition();

            glm::vec3 lightToCamera = cameraPos - lightPos;

            // dot product to get distance squared, less expensive than sqrt
            float distanceSq = glm::dot(lightToCamera, lightToCamera);

            sorted[distanceSq] = entity;
        }
        
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

        for (auto& [_, entity] : std::ranges::reverse_view(sorted))
        {
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