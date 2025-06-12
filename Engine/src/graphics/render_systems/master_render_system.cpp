#include "graphics/render_systems/master_render_system.hpp"

namespace PXTEngine {
	MasterRenderSystem::MasterRenderSystem(Context& context, Renderer& renderer, 
			Shared<DescriptorAllocatorGrowable> descriptorAllocator, 
			TextureRegistry& textureRegistry, MaterialRegistry& materialRegistry, 
			BLASRegistry& blasRegistry,
			Shared<DescriptorSetLayout> globalSetLayout,
			Shared<Environment> environment)
		:	m_context(context), 
			m_renderer(renderer),
			m_descriptorAllocator(std::move(descriptorAllocator)),
			m_textureRegistry(textureRegistry),
		    m_materialRegistry(materialRegistry),
			m_blasRegistry(blasRegistry),
			m_globalSetLayout(std::move(globalSetLayout)),
			m_environment(std::move(environment))
	{
		m_offscreenColorFormat = m_context.findSupportedFormat(
			{ VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R8G8B8A8_UNORM },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT |
			VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
			VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT
		);

		std::cout << "Offscreen color format: " << m_offscreenColorFormat << std::endl;

		if (m_offscreenColorFormat == VK_FORMAT_UNDEFINED) {
			throw std::runtime_error("Failed to find a suitable offscreen color format for MasterRenderSystem's render target!");
		}

		// to handle viewport resizing
		VkExtent2D swapChainExtent = m_renderer.getSwapChainExtent();
		m_lastFrameSwapChainExtent = swapChainExtent;

		createRenderPass();
		createSceneImage();
		createOffscreenDepthResources();
		createOffscreenFrameBuffer();
		createRenderSystems();
		
		createDescriptorSetsImGui();
	}

	MasterRenderSystem::~MasterRenderSystem() {
		vkDestroyFramebuffer(m_context.getDevice(), m_offscreenFb, nullptr);
		vkDestroyRenderPass(m_context.getDevice(), m_offscreenRenderPass, nullptr);
	};

	void MasterRenderSystem::recreateViewportResources() {
		// wait for the device to be idle
		vkDeviceWaitIdle(m_context.getDevice());

		// destroy old resources
		vkDestroyFramebuffer(m_context.getDevice(), m_offscreenFb, nullptr);

		VkExtent2D swapChainExtent = m_renderer.getSwapChainExtent();

		createSceneImage();
		createOffscreenDepthResources();
		createOffscreenFrameBuffer();

		updateImguiDescriptorSet();
	}

	void MasterRenderSystem::createRenderPass() {
		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = m_context.findDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = m_offscreenColorFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.dstSubpass = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.srcAccessMask = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

		// A second dependency for the transition to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		// This ensures that when the render pass finishes, the image is ready for sampling.
		VkSubpassDependency readDependency{};
		readDependency.srcSubpass = 0;
		readDependency.dstSubpass = VK_SUBPASS_EXTERNAL;
		readDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		readDependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // Read in fragment shader
		readDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		readDependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // Read by shader

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		std::array<VkSubpassDependency, 2> dependencies = { dependency, readDependency };

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		if (vkCreateRenderPass(m_context.getDevice(), &renderPassInfo, nullptr, &m_offscreenRenderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create offscreen render pass for MasterRenderSystem!");
		}
	}

	void MasterRenderSystem::createSceneImage() {
		VkExtent2D swapChainExtent = m_renderer.getSwapChainExtent();

		VkImageCreateInfo sceneImageInfo{};
		sceneImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		sceneImageInfo.imageType = VK_IMAGE_TYPE_2D;
		sceneImageInfo.extent.width = swapChainExtent.width;
		sceneImageInfo.extent.height = swapChainExtent.height;
		sceneImageInfo.extent.depth = 1;
		sceneImageInfo.mipLevels = 1;
		sceneImageInfo.arrayLayers = 1;
		sceneImageInfo.format = m_offscreenColorFormat;
		sceneImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		sceneImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		sceneImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | // to be writable in a renderpass
							   VK_IMAGE_USAGE_SAMPLED_BIT |			 // to be readable in a shader
							   VK_IMAGE_USAGE_STORAGE_BIT;			 // to be writable for raytracing shaders
		sceneImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		sceneImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		m_sceneImage = createShared<VulkanImage>(
			m_context,
			sceneImageInfo,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		// transition once to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL layout
		m_context.transitionImageLayoutSingleTimeCmd(
			m_sceneImage->getVkImage(),
			m_sceneImage->getImageFormat(),
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
		);

		VkImageViewCreateInfo colorViewInfo{};
		colorViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		colorViewInfo.image = m_sceneImage->getVkImage();
		colorViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		colorViewInfo.format = m_offscreenColorFormat;
		colorViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		colorViewInfo.subresourceRange.baseMipLevel = 0;
		colorViewInfo.subresourceRange.levelCount = 1;
		colorViewInfo.subresourceRange.baseArrayLayer = 0;
		colorViewInfo.subresourceRange.layerCount = 1;

		m_sceneImage->createImageView(colorViewInfo);

		// sampler for later imgui access
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		// Addressing Mode: Clamp to edge is usually best for render targets
		// This prevents "wrapping" artifacts if the sampling coordinates go slightly outside [0,1]
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = m_context.getPhysicalDeviceProperties().limits.maxSamplerAnisotropy;
		// Unnormalized coordinates: Use normalized UVs (0.0 to 1.0)
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		// Comparison: Not for texture sampling, leave disabled
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f; // Only use base mip level

		m_sceneImage->createSampler(samplerInfo);
	}

	void MasterRenderSystem::createOffscreenDepthResources() {
		// DEPTH RESOURCE
		VkFormat depthFormat = m_context.findDepthFormat();
		VkExtent2D swapChainExtent = m_renderer.getSwapChainExtent();

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = swapChainExtent.width;
		imageInfo.extent.height = swapChainExtent.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = depthFormat;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		m_offscreenDepthImage = createUnique<VulkanImage>(
			m_context,
			imageInfo,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_offscreenDepthImage->getVkImage();
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = depthFormat;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		m_offscreenDepthImage->createImageView(viewInfo);
	}

	void MasterRenderSystem::createOffscreenFrameBuffer() {
		// Create the offscreen framebuffer
		std::array<VkImageView, 2> attachments = { m_sceneImage->getImageView(), m_offscreenDepthImage->getImageView() };
		VkExtent2D swapChainExtent = m_renderer.getSwapChainExtent();

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_offscreenRenderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(m_context.getDevice(), &framebufferInfo, nullptr, &m_offscreenFb) != VK_SUCCESS) {
			throw std::runtime_error("failed to create offscreen framebuffer for MasterRenderSystem!");
		}
	}

	void MasterRenderSystem::createRenderSystems() {
		m_pointLightSystem = createUnique<PointLightSystem>(
			m_context,
			m_offscreenRenderPass,
			m_globalSetLayout->getDescriptorSetLayout()
		);

		m_shadowMapRenderSystem = createUnique<ShadowMapRenderSystem>(
			m_context,
			m_descriptorAllocator,
			*m_globalSetLayout
		);

		m_materialRenderSystem = createUnique<MaterialRenderSystem>(
			m_context,
			m_descriptorAllocator,
			m_textureRegistry,
			*m_globalSetLayout,
			m_offscreenRenderPass,
			m_shadowMapRenderSystem->getShadowMapImageInfo()
		);

		m_debugRenderSystem = createUnique<DebugRenderSystem>(
			m_context,
			m_descriptorAllocator,
			m_textureRegistry,
			m_offscreenRenderPass,
			*m_globalSetLayout
		);

		m_uiRenderSystem = createUnique<UiRenderSystem>(
			m_context,
			m_renderer.getSwapChainRenderPass()
		);

		m_skyboxRenderSystem = createUnique<SkyboxRenderSystem>(
			m_context,
			m_environment,
			*m_globalSetLayout,
			m_offscreenRenderPass
		);

		m_rayTracingRenderSystem = createUnique<RayTracingRenderSystem>(
			m_context,
			m_descriptorAllocator,
			m_textureRegistry,
			m_materialRegistry,
			m_blasRegistry,
			m_environment,
			*m_globalSetLayout,
			m_sceneImage
		);
	}

	void MasterRenderSystem::onUpdate(FrameInfo& frameInfo, GlobalUbo& ubo) {
		// check if viewport size has changed, if so recreate resources
		VkExtent2D swapChainExtent = m_renderer.getSwapChainExtent();
		if (swapChainExtent.width != m_lastFrameSwapChainExtent.width ||
			swapChainExtent.height != m_lastFrameSwapChainExtent.height) {
			recreateViewportResources();

			// update scene image for raytracing
			m_rayTracingRenderSystem->updateSceneImage(m_sceneImage);
			m_lastFrameSwapChainExtent = swapChainExtent;
		}
		
		// update ubo buffer
		ubo.projection = frameInfo.camera.getProjectionMatrix();
		ubo.view = frameInfo.camera.getViewMatrix();
		ubo.inverseView = frameInfo.camera.getInverseViewMatrix();

		// update light values into ubo
		m_pointLightSystem->update(frameInfo, ubo);

		// update shadow map
		m_shadowMapRenderSystem->update(frameInfo, ubo);

		// update raytracing scene
		if (m_isRaytracingEnabled) {
			m_rayTracingRenderSystem->update(frameInfo);
		}
	}

	void MasterRenderSystem::doRenderPasses(FrameInfo& frameInfo) {
		// begin new frame imgui
		m_uiRenderSystem->beginBuildingUi();

		// render to offscreen main render pass
		if (m_isRaytracingEnabled) {
			m_rayTracingRenderSystem->render(frameInfo, m_renderer);
			// this transitions the scene image back to shader_read_only_optimal for the next
			// renderpass (for now only point light billboards)
			m_rayTracingRenderSystem->transitionImageToShaderReadOnlyOptimal(frameInfo);

			//begin offscreen render pass for point light billboards
			/*m_renderer.beginRenderPass(frameInfo.commandBuffer, m_offscreenRenderPass,
				m_offscreenFb, m_renderer.getSwapChainExtent());

			m_pointLightSystem->render(frameInfo);

			m_renderer.endRenderPass(frameInfo.commandBuffer);*/
		} else {
			// render shadow cube map
			// the render function of the shadow map render system will
			// do how many passes it needs to do (6 in this case - 1 point light)
			m_shadowMapRenderSystem->render(frameInfo, m_renderer);

			//begin offscreen render pass
			m_renderer.beginRenderPass(frameInfo.commandBuffer, m_offscreenRenderPass,
				m_offscreenFb, m_renderer.getSwapChainExtent());

			m_skyboxRenderSystem->render(frameInfo);

			// choose if debug or not
			if (m_isDebugEnabled) {
				m_debugRenderSystem->render(frameInfo);
			}
			else {
				m_materialRenderSystem->render(frameInfo);
			}

			m_pointLightSystem->render(frameInfo);

			m_renderer.endRenderPass(frameInfo.commandBuffer);
		}

		// update scene ui
		this->updateUi();

		// render imgui and present
		m_renderer.beginSwapChainRenderPass(frameInfo.commandBuffer);

		// render ui and end imgui frame
		m_uiRenderSystem->render(frameInfo);

		m_renderer.endRenderPass(frameInfo.commandBuffer);
	}

	void MasterRenderSystem::createDescriptorSetsImGui() {
		// DESCRIPTOR SET FOR IMGUI VIEWPORT
		m_sceneDescriptorSetLayout = DescriptorSetLayout::Builder(m_context)
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
			.build();

		VkDescriptorImageInfo imageInfo;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = m_sceneImage->getImageView();
		imageInfo.sampler = m_sceneImage->getImageSampler();

		m_descriptorAllocator->allocate(m_sceneDescriptorSetLayout->getDescriptorSetLayout(), m_sceneDescriptorSet);

		DescriptorWriter(m_context, *m_sceneDescriptorSetLayout)
			.writeImage(0, &imageInfo)
			.updateSet(m_sceneDescriptorSet);
	}

	void MasterRenderSystem::updateImguiDescriptorSet() {
		VkDescriptorImageInfo imageInfo;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = m_sceneImage->getImageView();
		imageInfo.sampler = m_sceneImage->getImageSampler();

		DescriptorWriter(m_context, *m_sceneDescriptorSetLayout)
			.writeImage(0, &imageInfo)
			.updateSet(m_sceneDescriptorSet);
	}

	ImVec2 MasterRenderSystem::getImageSizeWithAspectRatioForImGuiWindow(
		ImVec2 windowSize, float aspectRatio) {
		ImVec2 ratioedExtent = { 0, 0 };

		// Calculate the width if the image fills the height
		float widthBasedOnHeight = windowSize.y * aspectRatio;

		// If filling the height makes the width exceed the window's width,
		// then the image must fill the width instead.
		if (widthBasedOnHeight > windowSize.x) {
			ratioedExtent.x = windowSize.x;
			ratioedExtent.y = windowSize.x / aspectRatio;
		}
		else {
			// Otherwise, fill the height
			ratioedExtent.x = widthBasedOnHeight;
			ratioedExtent.y = windowSize.y;
		}

		return ratioedExtent;
	}

	void MasterRenderSystem::updateSceneUi() {
		ImTextureID scene = (ImTextureID) m_sceneDescriptorSet;

		// we push a style var to remove the viewpoer window padding
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		// we see the size of the window and we make the image fit the window with an aspect ratio
		ImVec2 windowSize = ImGui::GetContentRegionAvail();
		m_sceneImageExtentInWindow = getImageSizeWithAspectRatioForImGuiWindow(
			windowSize,
			m_sceneImage->getAspectRatio()
		);

		// Calculate the horizontal and vertical offsets for centering
		float titleBarSize = ImGui::GetFrameHeight() * 2;
		float offsetX = (windowSize.x - m_sceneImageExtentInWindow.x) * 0.5f;
		float offsetY = (windowSize.y - m_sceneImageExtentInWindow.y + titleBarSize) * 0.5f;

		// Move the cursor to the calculated position
		// ImGui::SetCursorPos() sets the next drawing position relative to the top-left of the *content region*.
		ImGui::SetCursorPos(ImVec2(offsetX, offsetY));

		ImGui::Image(scene, m_sceneImageExtentInWindow);
		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::Begin("Raytracing Renderer");
		ImGui::Checkbox("Enable Raytracing", &m_isRaytracingEnabled);
		ImGui::End();

		ImGui::Begin("Debug Renderer");
		ImGui::Checkbox("Enable Debug", &m_isDebugEnabled);

		if (m_isDebugEnabled) {
			ImGui::Text("Debug Renderer is enabled");
			m_debugRenderSystem->updateUi();
		} else {
			ImGui::Text("Debug Renderer is disabled");
		}
		ImGui::End();
	}

	void MasterRenderSystem::updateUi() {
		updateSceneUi();

		m_shadowMapRenderSystem->updateUi();
	}
}