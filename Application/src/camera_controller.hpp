#include "cgengine.h"

using namespace CGEngine;

class CameraController : public Script {
    public:
        void onUpdate(float deltaTime) override;
        
    private:
        float m_moveSpeed{1.f};
        float m_lookSpeed{1.75f};

        glm::vec2 m_lastMousePos{0.f, 0.f};
        bool m_firstMouse = true;
        float m_mouseSensitivity = 0.0025f; // Adjust to taste
};