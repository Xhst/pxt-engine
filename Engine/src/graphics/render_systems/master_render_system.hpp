#pragma once

#include "core/memory.hpp"
#include "graphics/context/context.hpp"
#include "graphics/renderer.hpp"
#include "graphics/descriptors/descriptors.hpp"
#include "graphics/frame_info.hpp"
#include "scene/scene.hpp"

#include "graphics/render_systems/material_render_system.hpp"
#include "graphics/render_systems/shadow_map_render_system.hpp"
#include "graphics/render_systems/point_light_system.hpp"
#include "graphics/render_systems/ui_render_system.hpp"

namespace PXTEngine {

	class MasterRenderSystem {
	public:
		MasterRenderSystem(Context& context, Renderer& renderer, 
						   Shared<DescriptorAllocatorGrowable> descriptorAllocator, 
						   Shared<DescriptorSetLayout> globalSetLayout);

		~MasterRenderSystem();

		MasterRenderSystem(const MasterRenderSystem&) = delete;
		MasterRenderSystem& operator=(const MasterRenderSystem&) = delete;
		MasterRenderSystem(MasterRenderSystem&&) = delete;
		MasterRenderSystem& operator=(MasterRenderSystem&&) = delete;

		void onUpdate(FrameInfo& frameInfo, GlobalUbo& ubo);
		void doRenderPasses(FrameInfo& frameInfo);

	private:
		void createRenderSystems();

		Context& m_context;
		Renderer& m_renderer;

		Shared<DescriptorAllocatorGrowable> m_descriptorAllocator;

		Shared<DescriptorSetLayout> m_globalSetLayout{};

		std::array<Unique<Buffer>, SwapChain::MAX_FRAMES_IN_FLIGHT> m_uboBuffers;

		Unique<MaterialRenderSystem> m_materialRenderSystem{};
		Unique<PointLightSystem> m_pointLightSystem{};
		Unique<ShadowMapRenderSystem> m_shadowMapRenderSystem{};
		Unique<UiRenderSystem> m_uiRenderSystem{};
	};
}