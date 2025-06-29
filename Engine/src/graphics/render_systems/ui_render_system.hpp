#pragma once

#include "core/pch.hpp"
#include "graphics/swap_chain.hpp"
#include "graphics/context/context.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/descriptors/descriptors.hpp"
#include "scene/scene.hpp"

namespace PXTEngine {

	class UiRenderSystem {
	public:
		UiRenderSystem(Context& context, VkRenderPass renderPass);
		~UiRenderSystem();

		UiRenderSystem(const UiRenderSystem&) = delete;
		UiRenderSystem& operator=(const UiRenderSystem&) = delete;

		void beginBuildingUi();
		void render(FrameInfo& frameInfo);

	private:
		void initImGui(VkRenderPass& renderPass);
		VkDescriptorSet addImGuiTexture(VkSampler sampler, VkImageView imageView, VkImageLayout layout);

		void buildUi();

		Context& m_context;

		Unique<DescriptorAllocatorGrowable> m_imguiDescriptorAllocator;
		Unique<DescriptorPool> m_imGuiPool{};
	};
}