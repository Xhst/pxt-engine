#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace PXTEngine {

    class Camera {
    public:
        void setPerspective(float fovy, float aspect, float near, float far);
        void setOrthographic(float left, float right, float top, float bottom, float near, float far);

        void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
        void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
        void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

        const glm::mat4& getProjectionMatrix() const { return m_projectionMatrix; }
        const glm::mat4& getViewMatrix() const { return m_viewMatrix; }
        const glm::mat4& getInverseViewMatrix() const { return m_inverseViewMatrix; }

        const glm::vec3 getPosition() const { return glm::vec3(m_inverseViewMatrix[3]); }

    private:
        void updateViewMatrix(glm::vec3 u, glm::vec3 v, glm::vec3 w, glm::vec3 position);

        glm::mat4 m_projectionMatrix{1.f};
        glm::mat4 m_viewMatrix{1.f};
        glm::mat4 m_inverseViewMatrix{1.f};
    };

}