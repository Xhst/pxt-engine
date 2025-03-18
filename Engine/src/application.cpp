#include "application.hpp"

#include "core/memory.hpp"
#include "core/events/event_dispatcher.hpp"
#include "core/events/window_event.hpp"
#include "scene/ecs/component.hpp"
#include "scene/ecs/entity.hpp"
#include "scene/camera.hpp"
#include "graphics/render_systems/simple_render_system.hpp"
#include "graphics/render_systems/point_light_system.hpp"
#include "scene/script/script.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <chrono>

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

    Application* Application::Instance = nullptr;

    Application::Application() {
        Instance = this;

        m_globalPool = DescriptorPool::Builder(m_device)
                .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT * 2)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT * 2)
                .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
                .build();

        // to enable imgui functionality (FIGATA)
        initImGui(m_window, m_device);
    }

    Application::~Application() {
            for (auto& [_, system] : m_systems) {
                system->onShutdown();
                delete system;
            }

            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        };

    void Application::initImGui(Window& window, Device& device) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui_ImplGlfw_InitForVulkan(window.getBaseWindow(), true);
        ImGui_ImplVulkan_InitInfo initInfo{};
        initInfo.Instance = device.getVkInstance();
        initInfo.PhysicalDevice = device.getPhysicalDevice();
        initInfo.Device = device.getDevice();
        initInfo.QueueFamily = device.getGraphicsQueueFamily();
        initInfo.Queue = device.graphicsQueue();
        initInfo.RenderPass = m_renderer.getSwapChainRenderPass();
        initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        initInfo.PipelineCache = VK_NULL_HANDLE;
        initInfo.DescriptorPool = m_globalPool->getDescriptorPool();
        initInfo.Allocator = nullptr;
        initInfo.MinImageCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
        initInfo.ImageCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
        initInfo.CheckVkResultFn = nullptr;
        ImGui_ImplVulkan_Init(&initInfo);

        ImGui_ImplVulkan_CreateFontsTexture();
    }

    void Application::run() {
        std::vector<Unique<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);

        for (int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
            uboBuffers[i] = createUnique<Buffer>(
                m_device,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            );
            uboBuffers[i]->map();
        }

        auto globalSetLayout = DescriptorSetLayout::Builder(m_device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            DescriptorWriter(*globalSetLayout, *m_globalPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }

        m_window.setEventCallback(std::bind(&Application::onEvent, this, std::placeholders::_1));

        SimpleRenderSystem simpleRenderSystem{
            m_device,
            m_renderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout()
        };

        PointLightSystem pointLightSystem{
            m_device,
            m_renderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout()
        };

        Camera camera;
        
        auto currentTime = std::chrono::high_resolution_clock::now();
    
        m_scene.onStart();
        
        while (isRunning()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float elapsedTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;
            
            m_scene.onUpdate(elapsedTime);
            
            float aspect = m_renderer.getAspectRatio();

            Entity mainCameraEntity = m_scene.getMainCameraEntity();

            if (mainCameraEntity) {
                const auto& cameraComponent = mainCameraEntity.get<CameraComponent>();
                const auto& transform = mainCameraEntity.get<TransformComponent>();

                camera = cameraComponent.camera;
                camera.setViewYXZ(transform.translation, transform.rotation);

                camera.setPerspective(glm::radians(50.f), aspect, 0.1f, 100.f);
            }
            
            if (auto commandBuffer = m_renderer.beginFrame()) {
                int frameIndex = m_renderer.getFrameIndex();

                FrameInfo frameInfo = {
                    frameIndex,
                    elapsedTime,
                    commandBuffer,
                    camera,
                    globalDescriptorSets[frameIndex],
                    m_scene
                };

                // update
                GlobalUbo ubo{};
                ubo.projection = camera.getProjectionMatrix();
                ubo.view = camera.getViewMatrix();
                ubo.inverseView = camera.getInverseViewMatrix();

                // update light values into ubo
                pointLightSystem.update(frameInfo, ubo);

                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();
                
                // its not used yet
                for (auto& [_, system] : m_systems) {
                    system->onUpdate(elapsedTime);
                }

                // render 
                m_renderer.beginSwapChainRenderPass(commandBuffer);

                simpleRenderSystem.render(frameInfo);
                pointLightSystem.render(frameInfo);

                imGuiRenderUI(frameInfo);

                m_renderer.endSwapChainRenderPass(commandBuffer);
                m_renderer.endFrame();
            }
        }

        vkDeviceWaitIdle(m_device.getDevice());
    }

    bool Application::isRunning() {
        return !m_window.shouldClose() && m_running;
    }

    void Application::onEvent(Event& event) {
        EventDispatcher dispatcher(event);

        dispatcher.dispatch<WindowCloseEvent>([this](auto& event) {
            m_running = false;
        });

        for (auto& [_, system] : m_systems) {
            if (event.isHandled()) {
                break;
            }

            system->onEvent(event);
        }
    }

    void Application::imGuiRenderUI(FrameInfo& frameInfo) {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), frameInfo.commandBuffer);
    }

    Entity Application::createPointLight(float intensity, float radius, glm::vec3 color) {
        Entity entity = m_scene.createEntity("point_light")
            .add<PointLightComponent>(intensity)
            .add<TransformComponent>(glm::vec3{0.f, 0.f, 0.f}, glm::vec3{radius, 1.f, 1.f}, glm::vec3{0.0f, 0.0f, 0.0f})
            .add<ColorComponent>(color);

        return entity;
    }
    
}

int main() {

    try {
        auto app = PXTEngine::initApplication();

        app->run();

        delete app;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}