#include "core/memory.hpp"
#include "core/window.hpp"
#include "renderer/pipeline.hpp"

namespace CGEngine {

    class Engine {
    public:
        Engine();
        ~Engine() = default;

        void run();

    private:
        bool isRunning();

        Unique<Window> m_window;
        Pipeline pipeline{"../shaders/simple_shader.vert", "../shaders/simple_shader.frag"};
    };
}