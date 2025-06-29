#pragma once

#include "core/pch.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/swap_chain.hpp"
#include "graphics/context/context.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/descriptors/descriptors.hpp"
#include "graphics/resources/texture_registry.hpp"
#include "scene/scene.hpp"

namespace PXTEngine {
	enum RenderMode {
		Fill = 0,
		Wireframe = 1
	};

    class DebugRenderSystem {
    public:
        DebugRenderSystem(Context& context, Shared<DescriptorAllocatorGrowable> descriptorAllocator, TextureRegistry& textureRegistry, VkRenderPass renderPass, DescriptorSetLayout& globalSetLayout);
        ~DebugRenderSystem();

        DebugRenderSystem(const DebugRenderSystem&) = delete;
        DebugRenderSystem& operator=(const DebugRenderSystem&) = delete;

        void render(FrameInfo& frameInfo);
        void updateUi();
		void reloadShaders();

    private:
        void createPipelineLayout(DescriptorSetLayout& globalSetLayout);
        void createPipelines(bool useCompiledSpirvFiles = true);
        
        Context& m_context;
		TextureRegistry& m_textureRegistry;

        VkRenderPass m_renderPassHandle;
        Unique<Pipeline> m_pipelineWireframe;
		Unique<Pipeline> m_pipelineSolid;
        VkPipelineLayout m_pipelineLayout;

		Shared<DescriptorAllocatorGrowable> m_descriptorAllocator;

		int m_renderMode = Fill;

        bool m_isNormalColorEnabled = false;
		bool m_isAlbedoMapEnabled = true;
		bool m_isNormalMapEnabled = true;
		bool m_isAOMapEnabled = true;

        std::array<const std::string, 2> m_shaderFilePaths = {
            "debug_shader.vert",
            "debug_shader.frag"
        };
    };
}