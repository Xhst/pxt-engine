#pragma once

#include "core/pch.hpp"
#include "graphics/context/context.hpp"

namespace PXTEngine {

	struct ShaderGroupInfo {
		VkRayTracingShaderGroupTypeKHR type;
		std::vector<std::pair<VkShaderStageFlagBits, std::string>> stages;
	};

    /**
     * @struct RayTracingPipelineConfigInfo
     * @brief Configuration information for the RAYTRACING pipeline.
     *
     * This structure contains various settings and configurations for creating a Vulkan raytracing pipeline.
	 * Such as the shader groups, pipeline layout and max recursion depth.
     */
    struct RayTracingPipelineConfigInfo {
        RayTracingPipelineConfigInfo() = default;
        RayTracingPipelineConfigInfo(const RayTracingPipelineConfigInfo&) = delete;
        RayTracingPipelineConfigInfo& operator=(const RayTracingPipelineConfigInfo&) = delete;

		std::vector<ShaderGroupInfo> shaderGroups{};
        VkPipelineLayout pipelineLayout = nullptr;
        uint32_t maxPipelineRayRecursionDepth = 1;
    };

    /**
     * @struct RasterizationPipelineConfigInfo
     * @brief Configuration information for the GRAPHICS pipeline.
     *
     * This structure contains various settings and configurations for creating a Vulkan graphics pipeline.
     */
    struct RasterizationPipelineConfigInfo {
        RasterizationPipelineConfigInfo() = default;
        RasterizationPipelineConfigInfo(const RasterizationPipelineConfigInfo&) = delete;
        RasterizationPipelineConfigInfo& operator=(const RasterizationPipelineConfigInfo&) = delete;

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
        Pipeline(Context& context, const std::vector<std::string>& shaderFilePaths,
                 const RasterizationPipelineConfigInfo& configInfo);
		Pipeline(Context& context, const RayTracingPipelineConfigInfo& configInfo);
                 
        ~Pipeline();

        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;

        void bind(VkCommandBuffer commandBuffer);

        static void defaultPipelineConfigInfo(RasterizationPipelineConfigInfo& configInfo);
        static void enableAlphaBlending(RasterizationPipelineConfigInfo& configInfo);

		VkPipeline getHandle() const { return m_pipeline; }

       private:
        static std::vector<char> readFile(const std::string& filename);

        void createGraphicsPipeline(
            const std::vector<std::string>& shaderFilePaths,
            const RasterizationPipelineConfigInfo& configInfo);

		void createRayTracingPipeline(const RayTracingPipelineConfigInfo& configInfo);

        void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

        Context& m_context;
        VkPipeline m_pipeline;

        std::vector<VkShaderModule> m_shaderModules{};
		VkPipelineBindPoint m_pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    };
}