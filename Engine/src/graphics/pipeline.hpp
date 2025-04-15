#pragma once

#include <string>
#include <vector>

#include "graphics/context/context.hpp"

namespace PXTEngine {

    /**
     * @struct PipelineConfigInfo
     * @brief Configuration information for the graphics pipeline.
     *
     * This structure contains various settings and configurations for creating a Vulkan graphics pipeline.
     */
    struct PipelineConfigInfo {
        PipelineConfigInfo() = default;
        PipelineConfigInfo(const PipelineConfigInfo&) = delete;
        PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

        std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    /**
     * @class Pipeline
     * @brief Represents a Vulkan graphics pipeline.
     *
     * This class encapsulates the creation and management of a Vulkan graphics pipeline, including shader modules,
     * pipeline layout, and render pass. It provides methods for binding the pipeline to a command buffer.
     */
    class Pipeline {
       public:
        Pipeline(Context& context, const std::vector<std::pair<VkShaderStageFlagBits, std::string>>& shaderFilePaths,
                 const PipelineConfigInfo& configInfo);
                 
        ~Pipeline();

        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;

        void bind(VkCommandBuffer commandBuffer);

        static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
        static void enableAlphaBlending(PipelineConfigInfo& configInfo);

       private:
        static std::vector<char> readFile(const std::string& filename);

        void createGraphicsPipeline(
            const std::vector<std::pair<VkShaderStageFlagBits, std::string>>& shaderFilePaths,
            const PipelineConfigInfo& configInfo);

        void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

        Context& m_context;
        VkPipeline m_graphicsPipeline;

        std::vector<VkShaderModule> m_shaderModules{};
    };
}