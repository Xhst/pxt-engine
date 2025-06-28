#include "graphics/pipeline.hpp"

#include "core/diagnostics.hpp"
#include "graphics/resources/vk_mesh.hpp"
#include "graphics/resources/vk_shader.hpp"
#include "graphics/frame_info.hpp"
#include "core/constants.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

namespace PXTEngine {

    struct SpecializationData {
        int32_t maxLights;
    };

    Pipeline::Pipeline(Context& context, const std::vector<std::string>& shaderFilePaths,
                       const RasterizationPipelineConfigInfo& configInfo) : m_context(context) {
        createGraphicsPipeline(shaderFilePaths, configInfo);
    }

	Pipeline::Pipeline(Context& context, const RayTracingPipelineConfigInfo& configInfo)
        : m_context(context) {
		createRayTracingPipeline(configInfo);
		VulkanShader(m_context, SPV_SHADERS_PATH + "material_shader.vert.spv");
	}

	Pipeline::~Pipeline() {
		for (const auto shaderModule : m_shaderModules) {
			vkDestroyShaderModule(m_context.getDevice(), shaderModule, nullptr);
		}
        vkDestroyPipeline(m_context.getDevice(), m_pipeline, nullptr);
    }

    std::vector<char> Pipeline::readFile(const std::string& filename) {
        std::ifstream file{filename, std::ios::ate | std::ios::binary};

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file: " + filename);
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

	void Pipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		if (vkCreateShaderModule(m_context.getDevice(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}
	}

	void Pipeline::createGraphicsPipeline(
		const std::vector<std::string>& shaderFilePaths,
		const RasterizationPipelineConfigInfo& configInfo
	) {
		// Ensure that the pipeline layout and render pass are properly set.
		PXT_ASSERT(configInfo.pipelineLayout != nullptr,
			"Cannot create graphics pipeline: no pipelineLayout provided in config info");

		PXT_ASSERT(configInfo.renderPass != nullptr,
			"Cannot create graphics pipeline: no renderPass provided in config info");

		// --- SPECIALIZATION CONSTANT SETUP (if needed for all shaders) ---
		SpecializationData specializationData = { MAX_LIGHTS };

		VkSpecializationMapEntry mapEntries[1] = {
			{ 0, offsetof(SpecializationData, maxLights), sizeof(int32_t) }
		};

		VkSpecializationInfo specializationInfo{};
		specializationInfo.mapEntryCount = 1;
		specializationInfo.pMapEntries = mapEntries;
		specializationInfo.dataSize = sizeof(SpecializationData);
		specializationInfo.pData = &specializationData;

		// --- Prepare shader stages ---
		// Container to keep created shader stage infos.
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		// Container to contain vulkan shader wrappers (if they go out of scope before the pipeline is created,
		// the shader modules will be destroyed automatically).
		std::vector<Unique<VulkanShader>> shaders{ shaderFilePaths.size() };

		// Loop through each provided shader stage.
		for (int i = 0; i < shaderFilePaths.size(); i++) {
			const auto& filepath = shaderFilePaths[i];
			// to handle memory stuff atomatically
			shaders[i] = createUnique<VulkanShader>(m_context, filepath);

			VkPipelineShaderStageCreateInfo shaderStageCreateInfo = shaders[i]->GetShaderStageCreateInfo();
			shaderStageCreateInfo.pSpecializationInfo = &specializationInfo;

			shaderStages.push_back(shaderStageCreateInfo);
		}

		// --- Set up the vertex input state ---
		auto& bindingDescriptions = configInfo.bindingDescriptions;
		auto& attributeDescriptions = configInfo.attributeDescriptions;

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		// --- Create the graphics pipeline ---
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
		pipelineInfo.pViewportState = &configInfo.viewportInfo;
		pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
		pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
		pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
		pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
		pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;
		pipelineInfo.layout = configInfo.pipelineLayout;
		pipelineInfo.renderPass = configInfo.renderPass;
		pipelineInfo.subpass = configInfo.subpass;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  // Optional
		pipelineInfo.basePipelineIndex = -1;                // Optional

		if (vkCreateGraphicsPipelines(
			m_context.getDevice(),
			VK_NULL_HANDLE,
			1,
			&pipelineInfo,
			nullptr,
			&m_pipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}
	}

	void Pipeline::createRayTracingPipeline(const RayTracingPipelineConfigInfo& configInfo) {
		// --- Prepare shader stages ---
		// Containers to keep created shader stage infos and shader group infos.
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;

		// Loop each group
		for (const auto& group : configInfo.shaderGroups) {
			// Loop through each provided shader stage in the group
			// Prepare the shader group create info.
			VkRayTracingShaderGroupCreateInfoKHR shaderGroupInfo{};
			shaderGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
			shaderGroupInfo.type = group.type;
			// first we set them all unused
			shaderGroupInfo.generalShader = VK_SHADER_UNUSED_KHR;
			shaderGroupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
			shaderGroupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
			shaderGroupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
			shaderGroupInfo.pShaderGroupCaptureReplayHandle = nullptr; // Optional

			for (const auto& [stage, filepath] : group.stages) {
				// Read the shader binary code from the file.
				auto shaderCode = readFile(filepath);

				// Create the shader module.
				VkShaderModule shaderModule;
				createShaderModule(shaderCode, &shaderModule);
				m_shaderModules.push_back(shaderModule);

				// Prepare the shader stage create info.
				VkPipelineShaderStageCreateInfo shaderStageInfo{};
				shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				shaderStageInfo.stage = stage;
				shaderStageInfo.module = shaderModule;
				shaderStageInfo.pName = "main";
				shaderStageInfo.flags = 0;
				shaderStageInfo.pNext = nullptr;
				shaderStages.push_back(shaderStageInfo);

				uint32_t currentStageIndex = static_cast<uint32_t>(shaderStages.size() - 1);

				// then we set the correct shader index in the group
				switch (group.type) {
				case VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR:
					// For RGEN or MISS, there's only one shader in the group
					shaderGroupInfo.generalShader = currentStageIndex;
					break;
				case VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR:
					if (stage == VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR) {
						shaderGroupInfo.closestHitShader = currentStageIndex;
					}
					else if (stage == VK_SHADER_STAGE_ANY_HIT_BIT_KHR) {
						shaderGroupInfo.anyHitShader = currentStageIndex;
					}
					// Add other stages in the future (e.. intersection for custom primitive hit groups)
					/* else if (stage == VK_SHADER_STAGE_INTERSECTION_BIT_KHR) {
						shaderGroupInfo.intersectionShader = currentStageIndex;
					}
					*/
					break;
				case VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR:
					if (stage == VK_SHADER_STAGE_INTERSECTION_BIT_KHR) {
						shaderGroupInfo.intersectionShader = currentStageIndex;
					}
					else if (stage == VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR) {
						shaderGroupInfo.closestHitShader = currentStageIndex;
					}
					else if (stage == VK_SHADER_STAGE_ANY_HIT_BIT_KHR) {
						shaderGroupInfo.anyHitShader = currentStageIndex;
					}
					break;
				default:
					// Handle error or unsupported group type
					throw std::runtime_error("Unsupported shader group type in createRayTracingPipeline");
				}
			}

			shaderGroups.push_back(shaderGroupInfo);
		}

		VkRayTracingPipelineCreateInfoKHR pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.groupCount = static_cast<uint32_t>(shaderGroups.size());
		pipelineInfo.pGroups = shaderGroups.data();
		pipelineInfo.maxPipelineRayRecursionDepth = configInfo.maxPipelineRayRecursionDepth;
		pipelineInfo.layout = configInfo.pipelineLayout;
		// pipelineInfo.pLibraryInfo = ...; // For pipeline libraries
		// pipelineInfo.pLibraryInterface = ...; // For pipeline libraries
		// pipelineInfo.pDynamicState = ...; // For dynamic states

		if (vkCreateRayTracingPipelinesKHR(m_context.getDevice(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create ray tracing pipeline!");
		}

		m_pipelineBindPoint = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;

		// --- Clean up: Destroy the shader modules ---
		for (auto& shaderModule : m_shaderModules) {
			vkDestroyShaderModule(m_context.getDevice(), shaderModule, nullptr);
			shaderModule = VK_NULL_HANDLE;
		}
		m_shaderModules.clear();
	}

	void Pipeline::bind(VkCommandBuffer commandBuffer) {
        vkCmdBindPipeline(commandBuffer, m_pipelineBindPoint, m_pipeline);
    }

    void Pipeline::defaultPipelineConfigInfo(RasterizationPipelineConfigInfo& configInfo) {
        configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
        configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        configInfo.rasterizationInfo.lineWidth = 1.0f;
        configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
        configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
        configInfo.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
        configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional

        // TODO: MSAA
        configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
        configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        configInfo.multisampleInfo.minSampleShading = 1.0f;           // Optional
        configInfo.multisampleInfo.pSampleMask = nullptr;             // Optional
        configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
        configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

        configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        configInfo.viewportInfo.viewportCount = 1;
        configInfo.viewportInfo.pViewports = nullptr;
        configInfo.viewportInfo.scissorCount = 1;
        configInfo.viewportInfo.pScissors = nullptr;

        configInfo.colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
        configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
        configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
        configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
        configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
        configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
        configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

        configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
        configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
        configInfo.colorBlendInfo.attachmentCount = 1;
        configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
        configInfo.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
        configInfo.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
        configInfo.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
        configInfo.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

        configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        configInfo.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
        configInfo.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
        configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
        configInfo.depthStencilInfo.front = {};  // Optional
        configInfo.depthStencilInfo.back = {};   // Optional

        configInfo.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
        configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
        configInfo.dynamicStateInfo.flags = 0;

        configInfo.bindingDescriptions = VulkanMesh::getVertexBindingDescriptions();
        configInfo.attributeDescriptions = VulkanMesh::getVertexAttributeDescriptions();
    }

    void Pipeline::enableAlphaBlending(RasterizationPipelineConfigInfo& configInfo) {
        configInfo.colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT; 

        configInfo.colorBlendAttachment.blendEnable = VK_TRUE;
        configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;

        configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		// previously used value when rendering to the swapchain
		//configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

		// this value is needed when rendering the image to imgui using ImGui::Image
		// (see https://github.com/ocornut/imgui/issues/6569#issuecomment-2878782866)
        configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    }

}