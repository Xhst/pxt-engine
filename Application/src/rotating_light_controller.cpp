#include "rotating_light_controller.hpp"

void RotatingLightController::onCreate() {
    auto& transform = get<TransformComponent>();

    if (transform.translation.z == 0.0f) {
        m_baseAngle = 0.0f;
    }
    if (transform.translation.z == 0.5f) {
        m_baseAngle = 2.0f * glm::pi<float>()/ 3.0f;
    }
    if (transform.translation.z == -0.5f) {
        m_baseAngle = -2.0f * glm::pi<float>()/ 3.0f;
    }
}
void RotatingLightController::onUpdate(float deltaTime) {
    auto& transform = get<TransformComponent>();

    transform.translation.x = 0.5f * glm::cos(m_angle + m_baseAngle);
    transform.translation.z = 0.5f * glm::sin(m_angle + m_baseAngle);

    m_angle = glm::mod(m_angle + deltaTime, glm::two_pi<float>());
}

