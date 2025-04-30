#pragma once

#include "core/memory.hpp"
#include "graphics/swap_chain.hpp"
#include "graphics/context/context.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/descriptors/descriptors.hpp"
#include "scene/scene.hpp"

namespace PXTEngine {

	class UiRenderSystem {
	public:
		UiRenderSystem(Context& context, VkRenderPass renderPass, VkDescriptorImageInfo shadowMapImageInfo);
		~UiRenderSystem();

		UiRenderSystem(const UiRenderSystem&) = delete;
		UiRenderSystem& operator=(const UiRenderSystem&) = delete;

		void render(FrameInfo& frameInfo);

	private:
		void initImGui(VkRenderPass& renderPass);
		void createDescriptorSets(VkDescriptorImageInfo sceneImageInfo);
		void buildUi();

		Context& m_context;

		Unique<DescriptorPool> m_imGuiPool{};

		VkDescriptorSet m_sceneDescriptorSet{};
	};
}