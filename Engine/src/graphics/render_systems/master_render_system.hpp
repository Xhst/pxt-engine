#pragma once

#include "core/memory.hpp"
#include "graphics/context/context.hpp"
#include "graphics/renderer.hpp"
#include "graphics/descriptors/descriptors.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/resources/texture_registry.hpp"
#include "graphics/resources/material_registry.hpp"
#include "graphics/resources/blas_registry.hpp"

#include "graphics/render_systems/material_render_system.hpp"
#include "graphics/render_systems/shadow_map_render_system.hpp"
#include "graphics/render_systems/point_light_system.hpp"
#include "graphics/render_systems/ui_render_system.hpp"
#include "graphics/render_systems/debug_render_system.hpp"
#include "graphics/render_systems/skybox_render_system.hpp"
#include "graphics/render_systems/raytracing_render_system.hpp"
#include "graphics/render_pass.hpp"
#include "graphics/frame_buffer.hpp"

#include "scene/environment.hpp"


namespace PXTEngine {

	class MasterRenderSystem {
	public:
		MasterRenderSystem(Context& context, Renderer& renderer, 
						   Shared<DescriptorAllocatorGrowable> descriptorAllocator,
						   TextureRegistry& textureRegistry,
						   MaterialRegistry& materialRegistry,
						   BLASRegistry& blasRegistry,
						   Shared<DescriptorSetLayout> globalSetLayout,
						   Shared<Environment> environment);

		~MasterRenderSystem();

		MasterRenderSystem(const MasterRenderSystem&) = delete;
		MasterRenderSystem& operator=(const MasterRenderSystem&) = delete;
		MasterRenderSystem(MasterRenderSystem&&) = delete;
		MasterRenderSystem& operator=(MasterRenderSystem&&) = delete;

		void onUpdate(FrameInfo& frameInfo, GlobalUbo& ubo);
		void doRenderPasses(FrameInfo& frameInfo);

	private:
		void recreateViewportResources();
		void createRenderPass();
		void createSceneImage();
		void createOffscreenDepthResources();
		void createOffscreenFrameBuffer();
		void createRenderSystems();

		void createDescriptorSetsImGui();
		void updateImguiDescriptorSet();

		ImVec2 getImageSizeWithAspectRatioForImGuiWindow(ImVec2 windowSize, float aspectRatio);
		void updateSceneUi();
		void updateUi();

		Context& m_context;
		Renderer& m_renderer;
		TextureRegistry& m_textureRegistry;
		MaterialRegistry& m_materialRegistry;
		BLASRegistry& m_blasRegistry;

		Shared<DescriptorAllocatorGrowable> m_descriptorAllocator;

		Shared<DescriptorSetLayout> m_globalSetLayout{};

		Shared<Environment> m_environment;

		std::array<Unique<VulkanBuffer>, SwapChain::MAX_FRAMES_IN_FLIGHT> m_uboBuffers;

		Unique<MaterialRenderSystem> m_materialRenderSystem = nullptr;
		Unique<PointLightSystem> m_pointLightSystem = nullptr;
		Unique<ShadowMapRenderSystem> m_shadowMapRenderSystem = nullptr;
		Unique<UiRenderSystem> m_uiRenderSystem = nullptr;
		Unique<DebugRenderSystem> m_debugRenderSystem = nullptr;
		Unique<SkyboxRenderSystem> m_skyboxRenderSystem = nullptr;
		Unique<RayTracingRenderSystem> m_rayTracingRenderSystem = nullptr;

		Unique<RenderPass> m_offscreenRenderPass;
		Unique<FrameBuffer> m_offscreenFb;

		Shared<VulkanImage> m_sceneImage;
		VkFormat m_offscreenColorFormat;
		Shared<VulkanImage> m_offscreenDepthImage;

		VkDescriptorSet m_sceneDescriptorSet = VK_NULL_HANDLE;
		Unique<DescriptorSetLayout> m_sceneDescriptorSetLayout = nullptr;

		VkExtent2D m_lastFrameSwapChainExtent;
		ImVec2 m_sceneImageExtentInWindow = { 960, 540 };

		bool m_isDebugEnabled = false;
		bool m_isRaytracingEnabled = true;
		bool m_isAccumulationEnabled = false;
	};
}