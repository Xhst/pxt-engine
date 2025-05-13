#include "graphics/render_systems/master_render_system.hpp"

namespace PXTEngine {
	MasterRenderSystem::MasterRenderSystem(Context& context, Renderer& renderer, 
			Shared<DescriptorAllocatorGrowable> descriptorAllocator, 
			TextureRegistry& textureRegistry, BLASRegistry& blasRegistry,
			Shared<DescriptorSetLayout> globalSetLayout)
		:	m_context(context), 
			m_renderer(renderer),
			m_descriptorAllocator(std::move(descriptorAllocator)),
			m_textureRegistry(textureRegistry),
			m_blasRegistry(blasRegistry),
			m_globalSetLayout(std::move(globalSetLayout)) {

		createRenderSystems();
	}

	MasterRenderSystem::~MasterRenderSystem() {};

	void MasterRenderSystem::createRenderSystems() {
		m_pointLightSystem = createUnique<PointLightSystem>(
			m_context,
			m_renderer.getSwapChainRenderPass(),
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
			m_renderer.getSwapChainRenderPass(),
			*m_globalSetLayout,
			m_shadowMapRenderSystem->getShadowMapImageInfo()
		);

		m_debugRenderSystem = createUnique<DebugRenderSystem>(
			m_context,
			m_descriptorAllocator,
			m_textureRegistry,
			m_renderer.getSwapChainRenderPass(),
			*m_globalSetLayout
		);

		m_uiRenderSystem = createUnique<UiRenderSystem>(
			m_context,
			m_renderer.getSwapChainRenderPass(),
			m_shadowMapRenderSystem->getDebugShadowMapImageInfos(),
			// TODO: replace with scene image info
			m_shadowMapRenderSystem->getShadowMapImageInfo()
		);

		m_rayTracingRenderSystem = createUnique<RayTracingRenderSystem>(
			m_context,
			m_descriptorAllocator,
			m_textureRegistry,
			m_blasRegistry,
			*m_globalSetLayout
		);
	}

	void MasterRenderSystem::onUpdate(FrameInfo& frameInfo, GlobalUbo& ubo) {
		// update ubo buffer
		ubo.projection = frameInfo.camera.getProjectionMatrix();
		ubo.view = frameInfo.camera.getViewMatrix();
		ubo.inverseView = frameInfo.camera.getInverseViewMatrix();

		// update light values into ubo
		m_pointLightSystem->update(frameInfo, ubo);

		// update shadow map
		m_shadowMapRenderSystem->update(frameInfo, ubo);

		// update raytracing scene
		m_rayTracingRenderSystem->update(frameInfo);
	}

	void MasterRenderSystem::doRenderPasses(FrameInfo& frameInfo) {
		// begin new frame imgui
		m_uiRenderSystem->beginBuildingUi();

		this->updateUi();

		// render shadow cube map
		// the render function of the shadow map render system will
		// do how many passes it needs to do (6 in this case - 1 point light)
		m_shadowMapRenderSystem->render(frameInfo, m_renderer);
		m_shadowMapRenderSystem->updateUi();

		// render main frame
		m_renderer.beginSwapChainRenderPass(frameInfo.commandBuffer);

		// choose if debug or not
		if (m_isDebugEnabled) {
			m_debugRenderSystem->render(frameInfo);
		}
		else {
			m_materialRenderSystem->render(frameInfo);
		}
		
		m_pointLightSystem->render(frameInfo);

		// render ui and end imgui frame
		m_uiRenderSystem->render(frameInfo);

		m_renderer.endRenderPass(frameInfo.commandBuffer);
	}

	void MasterRenderSystem::updateUi() {
		ImGui::Begin("Debug Renderer");

		ImGui::Checkbox("Enable Debug", &m_isDebugEnabled);
		
		if (m_isDebugEnabled) {
			ImGui::Text("Debug Renderer is enabled");
			m_debugRenderSystem->updateUi();
		}
		else {
			ImGui::Text("Debug Renderer is disabled");
		}

		ImGui::End();
	}
}