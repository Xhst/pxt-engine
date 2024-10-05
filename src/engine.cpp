#include "engine.hpp"
#include "core/memory.hpp"
#include "scene/ecs/entity.hpp"


#include <iostream>
#include <array>
#include <stdexcept>
#include <cassert>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace CGEngine {

    struct SimplePushConstantData {
        glm::mat2 transform{1.f};
        glm::vec2 offset;
        alignas(16) glm::vec3 color;
    };

    Engine::Engine() {
        loadScene();
        createPipelineLayout();
        recreateSwapChain();
        createCommandBuffers();
    }

    Engine::~Engine() {
        vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
    }

    void Engine::run() {
        while (isRunning()) {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(m_device.device());
    }

    bool Engine::isRunning() {
        return !m_window.shouldClose();
    }

    void Engine::loadScene() {
        std::vector<Model::Vertex> vertices{
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };
        auto model = createShared<Model>(m_device, vertices);

        m_scene = createUnique<Scene>();

        auto rotation = .25f * glm::two_pi<float>();
        m_scene->createEntity("triangle")
            .add<Transform2dComponent>(glm::vec2{0.5f, 0.0f}, glm::vec2{0.5f, 0.5f}, rotation)
            .add<ColorComponent>(glm::vec3{0.6f, 0.0f, 0.5f})
            .add<ModelComponent>(model);
    }

    void Engine::createPipelineLayout() {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void Engine::createPipeline() {
        assert(m_swapChain != nullptr && "Cannot create pipeline before swap chain");
        assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipelineLayout");

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = m_swapChain->getRenderPass();
        pipelineConfig.pipelineLayout = m_pipelineLayout;

        m_pipeline = createUnique<Pipeline>(
            m_device,
            "../shaders/simple_shader.vert.spv",
            "../shaders/simple_shader.frag.spv",
            pipelineConfig
        );
    }

    void Engine::recreateSwapChain() {
        auto extent = m_window.getExtent();
        while (extent.width == 0 || extent.height == 0) {
            extent = m_window.getExtent();
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(m_device.device());

        if (m_swapChain == nullptr) {
            m_swapChain = createUnique<SwapChain>(m_device, extent);
        } else {
            m_swapChain = createUnique<SwapChain>(m_device, extent, std::move(m_swapChain));
            if (m_swapChain->imageCount() != m_commandBuffers.size()) {
                freeCommandBuffers();
                createCommandBuffers();
            }
        }
        createPipeline();
    }

    void Engine::createCommandBuffers() {
        m_commandBuffers.resize(m_swapChain->imageCount());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_device.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

        if (vkAllocateCommandBuffers(m_device.device(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void Engine::freeCommandBuffers() {
        vkFreeCommandBuffers(
            m_device.device(),
            m_device.getCommandPool(),
            static_cast<uint32_t>(m_commandBuffers.size()),
            m_commandBuffers.data());
        
        m_commandBuffers.clear();
    }

    void Engine::recordCommandBuffer(int imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(m_commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_swapChain->getRenderPass();
        renderPassInfo.framebuffer = m_swapChain->getFrameBuffer(imageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_swapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(m_commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_swapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(m_swapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, m_swapChain->getSwapChainExtent()};
        vkCmdSetViewport(m_commandBuffers[imageIndex], 0, 1, &viewport);
        vkCmdSetScissor(m_commandBuffers[imageIndex], 0, 1, &scissor);

        renderScene(m_commandBuffers[imageIndex]);

        vkCmdEndRenderPass(m_commandBuffers[imageIndex]);

        if (vkEndCommandBuffer(m_commandBuffers[imageIndex]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void Engine::renderScene(VkCommandBuffer commandBuffer) {
        m_pipeline->bind(commandBuffer);

        auto view = m_scene->getEntitiesWith<Transform2dComponent, ColorComponent, ModelComponent>();
        for (auto entity : view) {

            const auto&[transform, color, model] = view.get<Transform2dComponent, ColorComponent, ModelComponent>(entity);

            transform.rotation = glm::mod(transform.rotation + 0.01f, glm::two_pi<float>());

            SimplePushConstantData push{};
            push.transform = transform;
            push.offset = transform.translation;
            push.color = color;

            vkCmdPushConstants(
                commandBuffer,
                m_pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(SimplePushConstantData),
                &push);

            auto modelPtr = model.model;
            
            modelPtr->bind(commandBuffer);
            modelPtr->draw(commandBuffer);

        }
    }

    void Engine::drawFrame() {
        uint32_t imageIndex;
        VkResult result = m_swapChain->acquireNextImage(&imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        recordCommandBuffer(imageIndex);

        result = m_swapChain->submitCommandBuffers(&m_commandBuffers[imageIndex], &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.isWindowResized()) {
            m_window.resetWindowResizedFlag();
            recreateSwapChain();
            return;
        } 

        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
    }

}