#include "core/memory.hpp"
#include "core/window.hpp"

namespace CGEngine {

    class Engine {
    public:
        Engine();
        ~Engine() = default;

        void run();

    private:
        bool isRunning();

        Unique<Window> m_window;
    };
}