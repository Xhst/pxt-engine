#include "graphics/render_systems/master_render_system.hpp"

namespace PXTEngine {
	MasterRenderSystem::MasterRenderSystem(Context& context, Renderer& renderer, 
			Shared<DescriptorAllocatorGrowable> descriptorAllocator, Shared<DescriptorSetLayout> globalSetLayout)
		:	m_context(context), 
			m_renderer(renderer),
			m_descriptorAllocator(std::move(descriptorAllocator)),
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
			m_renderer.getSwapChainRenderPass(),
			*m_globalSetLayout,
			m_shadowMapRenderSystem->getShadowMapImageInfo()
		);

		m_uiRenderSystem = createUnique<UiRenderSystem>(
			m_context,
			m_renderer.getSwapChainRenderPass(),
			m_shadowMapRenderSystem->getShadowMapImageInfo()
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
	}

	void MasterRenderSystem::doRenderPasses(FrameInfo& frameInfo) {
		// render shadow cube map
		// the render function of the shadow map render system will
		// do how many passes it needs to do (6 in this case - 1 point light)
		m_shadowMapRenderSystem->render(frameInfo, m_renderer);

		// render main frame
		m_renderer.beginSwapChainRenderPass(frameInfo.commandBuffer);

		m_materialRenderSystem->render(frameInfo);
		m_pointLightSystem->render(frameInfo);

		m_uiRenderSystem->render(frameInfo);

		m_renderer.endRenderPass(frameInfo.commandBuffer);
	}
}