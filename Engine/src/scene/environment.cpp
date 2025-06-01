#include "scene/environment.hpp"

#include "graphics/resources/skybox.hpp"

namespace PXTEngine {

	void Environment::setSkybox(const std::array<std::string, 6>& skyboxTextures) {
		m_skybox = VulkanSkybox::create(skyboxTextures);
	}
}