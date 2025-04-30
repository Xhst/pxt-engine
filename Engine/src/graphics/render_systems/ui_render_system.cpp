#include "graphics/render_systems/ui_render_system.hpp"
#include "core/memory.hpp"
#include "core/error_handling.hpp"
#include "core/constants.hpp"

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

	UiRenderSystem::UiRenderSystem(Context& context, VkRenderPass renderPass, VkDescriptorImageInfo sceneImageInfo) : m_context(context) {
		initImGui(renderPass);
		createDescriptorSets(sceneImageInfo);
	}

	UiRenderSystem::~UiRenderSystem() {
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void UiRenderSystem::initImGui(VkRenderPass& renderPass) {
		m_imGuiPool = DescriptorPool::Builder(m_context)
			.setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
			// TODO: try to make the pool dynamic
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT * 2) // fonts and scene texture
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
		initInfo.RenderPass = renderPass;
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

	void UiRenderSystem::createDescriptorSets(VkDescriptorImageInfo sceneImageInfo) {
		m_sceneDescriptorSet = ImGui_ImplVulkan_AddTexture(
			sceneImageInfo.sampler,
			sceneImageInfo.imageView,
			sceneImageInfo.imageLayout
		);
	}

	void UiRenderSystem::render(FrameInfo& frameInfo) {
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		buildUi();

		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), frameInfo.commandBuffer);
	}

	void UiRenderSystem::buildUi() {
		ImGui::ShowMetricsWindow();
	}
}