#include "camera_controller.hpp"

void CameraController::onUpdate(float deltaTime) {
    auto& transform = get<TransformComponent>();

    glm::vec3 rotate{0};
    if (Input::isKeyPressed(KeyCode::RightArrow)) rotate.y += 1.f;
    if (Input::isKeyPressed(KeyCode::LeftArrow)) rotate.y -= 1.f;
    if (Input::isKeyPressed(KeyCode::UpArrow)) rotate.x += 1.f;
    if (Input::isKeyPressed(KeyCode::DownArrow)) rotate.x -= 1.f;

    if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
        transform.rotation += m_lookSpeed * deltaTime * glm::normalize(rotate);
    }

    transform.rotation.x = glm::clamp(transform.rotation.x, -1.5f, 1.5f);
    transform.rotation.y = glm::mod(transform.rotation.y, glm::two_pi<float>());

    float yaw = transform.rotation.y;
    const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
    const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
    const glm::vec3 upDir{0.f, -1.f, 0.f};

    glm::vec3 moveDir{0.f};
    if (Input::isKeyPressed(KeyCode::W)) moveDir += forwardDir;
    if (Input::isKeyPressed(KeyCode::S)) moveDir -= forwardDir;
    if (Input::isKeyPressed(KeyCode::D)) moveDir += rightDir;
    if (Input::isKeyPressed(KeyCode::A)) moveDir -= rightDir;
    if (Input::isKeyPressed(KeyCode::E)) moveDir += upDir;
    if (Input::isKeyPressed(KeyCode::Q)) moveDir -= upDir;

    if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
        transform.translation += m_moveSpeed * deltaTime * glm::normalize(moveDir);
    }
}
        
