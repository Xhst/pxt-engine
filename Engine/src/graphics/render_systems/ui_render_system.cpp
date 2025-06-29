#include "graphics/render_systems/ui_render_system.hpp"

namespace PXTEngine {

	UiRenderSystem::UiRenderSystem(Context& context, VkRenderPass renderPass) : m_context(context) {
		initImGui(renderPass);
	}

	UiRenderSystem::~UiRenderSystem() {
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void UiRenderSystem::initImGui(VkRenderPass& renderPass) {
		// we need one set per imgui rendered texture NOT PER FRAME!!! (fonts included)
		// ImGui will use the same descriptor set for all textures.
		// ImGui will use this pool for fonts and its stuff, textures will be allocated from the descriptor allocator growable
		// TODO: maybe set format will change
		m_imGuiPool = DescriptorPool::Builder(m_context)
			.setMaxSets(2) 
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2)
			.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
			.build();

		std::vector<PoolSizeRatio> poolRatios = {
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5.0f },
		};

		m_imguiDescriptorAllocator = createUnique<DescriptorAllocatorGrowable>(
			m_context,
			8,       // starting sets per pool
			poolRatios,
			2.0f,     // growth factor
			512       // max sets cap
		);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		// enable docking and load ini file
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		io.IniFilename = IMGUI_INI_FILEPATH.c_str();
		PXT_INFO("ImGui .ini file set to: {}", io.IniFilename);

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
		initInfo.CheckVkResultFn = [](VkResult err) {
			if (err != VK_SUCCESS) {
				std::cerr << "[Vulkan Error] VkResult: " << err << std::endl;
				assert(false);
			}
		};
		ImGui_ImplVulkan_Init(&initInfo);

		ImGui_ImplVulkan_CreateFontsTexture();
	}

	VkDescriptorSet UiRenderSystem::addImGuiTexture(VkSampler sampler, VkImageView imageView, VkImageLayout layout) {
		VkDescriptorSet descriptorSet;
		
		Unique<DescriptorSetLayout> imguiLayout = DescriptorSetLayout::Builder(m_context)
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		m_imguiDescriptorAllocator->allocate(imguiLayout->getDescriptorSetLayout(), descriptorSet);

		VkDescriptorImageInfo descImage{};
		descImage.sampler = sampler;
		descImage.imageView = imageView;
		descImage.imageLayout = layout;

		DescriptorWriter(m_context, *imguiLayout)
			.writeImage(0, &descImage)
			.updateSet(descriptorSet);

		return descriptorSet;
	}

	void UiRenderSystem::render(FrameInfo& frameInfo) {
		buildUi();

		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), frameInfo.commandBuffer);
	}

	void UiRenderSystem::beginBuildingUi() {
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// MAIN MENU BAR (might become a method itself in the future)
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Open...")) {
					// TODO: Implement "Open" logic here
					printf("File -> Open... clicked!\n");
				}
				if (ImGui::MenuItem("Exit")) {
					// TODO: Implement "Exit" logic here
					printf("File -> Exit clicked!\n");
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		// IMPORTANT: This is required for docking to work in the main window (for customizations, view imgui_demo.cpp)
		ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
	}

	void UiRenderSystem::buildUi() {
		// TODO: add ImGui windows
		ImGui::ShowMetricsWindow();
	}
}