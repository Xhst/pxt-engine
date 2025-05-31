#pragma once

#include "graphics/resources/cube_map.hpp"

namespace PXTEngine {

	class Skybox {
	public:
		Skybox(Context& context, const std::array<std::string, 6>& paths);
		~Skybox() = default;

		VkDescriptorImageInfo getDescriptorImageInfo() const;

	private:
		void loadTextures(const std::array<std::string, 6>& paths);

		Context& m_context;
		
		uint32_t m_size = 0;
		Unique<CubeMap> m_cubeMap;
	};
}