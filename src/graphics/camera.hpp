#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace CGEngine {

    class Camera {
    public:
        void setPerspective(float fovy, float aspect, float near, float far);
        void setOrthographic(float left, float right, float top, float bottom, float near, float far);

        const glm::mat4& getProjectionMatrix() const { return m_projectionMatrix; }

    private:
        glm::mat4 m_projectionMatrix{1.f};
    };
    
}