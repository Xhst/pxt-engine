#pragma once

#include "application.hpp"
#include "core/input/key_code.hpp"
#include "core/input/mapper/glfw_input_mapper.hpp"

#include <glm/glm.hpp>

namespace PXTEngine {

    /**
     * @class Input
     * @brief Class for handling input events.
     * 
     * The Input class provides static methods for querying input events such as key presses, 
     * mouse button presses, and mouse movement.
     */
    class Input {
    public:
        /**
         * @brief Checks if a key is released.
         * 
         * @param key The key to check.
         * @return True if the key is released, false otherwise.
         */
        static bool isKeyReleased(KeyCode key) {
            return glfwGetKey(getWindow(), mapToGLFWKey(key)) == GLFW_RELEASE;
        }
        
        /**
         * @brief Checks if a key is currently pressed.
         * 
         * @param key The key to check.
         * @return True if the key is currently pressed, false otherwise.
         */
        static bool isKeyPressed(KeyCode key) {
            return glfwGetKey(getWindow(), mapToGLFWKey(key)) == GLFW_PRESS;
        }

        /**
         * @brief Checks if a key is being repeated (held down).
         * 
         * @param key The key to check.
         * @return True if the key is being repeated, false otherwise.
         */
        static bool isKeyRepeated(KeyCode key) {
            return glfwGetKey(getWindow(), mapToGLFWKey(key)) == GLFW_REPEAT;
        }

        /**
         * @brief Checks if a mouse button is pressed.
         * 
         * @param button The mouse button to check.
         * @return True if the mouse button is pressed, false otherwise.
         */
        static bool isMouseButtonPressed(MouseButton button) {
            return glfwGetMouseButton(getWindow(), mapToGLFWMouseButton(button)) == GLFW_PRESS;
        }

        /**
         * @brief Gets the current mouse position.
         * 
         * @return The current mouse position.
         */
        static glm::vec2 getMousePosition() {
            double x, y;
		    glfwGetCursorPos(getWindow(), &x, &y);
            return { x, y };
        }

    private:
        static GLFWwindow* getWindow() {
            return Application::get().getWindow().getBaseWindow();
        }
    };
}