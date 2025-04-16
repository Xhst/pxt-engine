#include "graphics/render_systems/master_render_system.hpp"

// IMGUI
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imconfig.h"
#include "imgui_tables.cpp"
#include "imgui_internal.h"
#include "imgui.h"
#include "imgui_draw.cpp"
#include "imgui_widgets.cpp"
#include "imgui_demo.cpp"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

namespace PXTEngine {
	MasterRenderSystem::MasterRenderSystem(Context& context, Renderer& renderer, 
			Shared<DescriptorAllocatorGrowable> descriptorAllocator, Shared<DescriptorSetLayout> globalSetLayout)
		:	m_context(context), 
			m_renderer(renderer),
			m_descriptorAllocator(std::move(descriptorAllocator)),
			m_globalSetLayout(std::move(globalSetLayout)) {

		createRenderSystems();
	}

	MasterRenderSystem::~MasterRenderSystem() {
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	};

	void MasterRenderSystem::createRenderSystems() {
		m_pointLightSystem = createUnique<PointLightSystem>(
			m_context,
			m_renderer.getSwapChainRenderPass(),
			m_globalSetLayout->getDescriptorSetLayout()
		);

		m_shadowMapRenderSystem = createUnique<ShadowMapRenderSystem>(
			m_context,
			m_descriptorAllocator,
			*m_globalSetLayout,
			VK_FORMAT_D16_UNORM
		);

		m_materialRenderSystem = createUnique<MaterialRenderSystem>(
			m_context,
			m_descriptorAllocator,
			m_renderer.getSwapChainRenderPass(),
			*m_globalSetLayout,
			m_shadowMapRenderSystem->getShadowMapImageInfo()
		);

		// to enable imGui functionality
		initImGui();
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

		imGuiRenderUI(frameInfo);

		m_renderer.endRenderPass(frameInfo.commandBuffer);
	}

	void MasterRenderSystem::initImGui() {
		m_imGuiPool = DescriptorPool::Builder(m_context)
			.setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT)
			.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
			.build();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImGui_ImplGlfw_InitForVulkan(m_context.getWindow().getBaseWindow(), true);
		ImGui_ImplVulkan_InitInfo initInfo{};
		initInfo.Instance = m_context.getInstance();
		initInfo.PhysicalDevice = m_context.getPhysicalDevice();
		initInfo.Device = m_context.getDevice();
		initInfo.QueueFamily = m_context.findPhysicalQueueFamilies().graphicsFamily;
		initInfo.Queue = m_context.getGraphicsQueue();
		initInfo.RenderPass = m_renderer.getSwapChainRenderPass();
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.PipelineCache = VK_NULL_HANDLE;
		initInfo.DescriptorPool = m_imGuiPool->getDescriptorPool();
		initInfo.Allocator = nullptr;
		initInfo.MinImageCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
		initInfo.ImageCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
		initInfo.CheckVkResultFn = nullptr;
		ImGui_ImplVulkan_Init(&initInfo);

		ImGui_ImplVulkan_CreateFontsTexture();
	}

	void MasterRenderSystem::imGuiRenderUI(const FrameInfo& frameInfo) {
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow();

		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), frameInfo.commandBuffer);
	}
}