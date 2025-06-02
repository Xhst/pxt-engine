#pragma once

#include "core/memory.hpp"
#include "scene/skybox.hpp"

#include <glm/glm.hpp>  

namespace PXTEngine {

	class Environment {
	public:

		/**
		 * @brief Get the ambient light of the environment.
		 * The RGB components represents the ambient light color, the alpha component represents the intensity.
		 *
		 * @return The ambient light.
		 */
		glm::vec4 getAmbientLight() const { return m_ambientLight; }

		/**
		 * @brief Set the ambient light of the environment.
		 * The RGB components represents the ambient light color, the alpha component represents the intensity.
		 *
		 * @param ambientLight The ambient light to set.
		 */
		void setAmbientLight(const glm::vec4& ambientLight) { m_ambientLight = ambientLight; }

		Shared<Skybox>& getSkybox() { return m_skybox; }

		/**
		 * @brief Set the skybox of the environment.
		 * The skybox is used to render the background of the scene.
		 *
		 * @param skyboxTextures An array of texture paths for the skybox faces.
		 */
		void setSkybox(const std::array<std::string, 6>& skyboxTextures);

	private:
		glm::vec4 m_ambientLight = glm::vec4{ 0.67f, 0.85f, 0.9f, .02f };

		Shared<Skybox> m_skybox = nullptr; 
	};
}