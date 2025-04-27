#pragma once

#include "core/memory.hpp"
#include "resources/resource.hpp"
#include "resources/types/image.hpp"

namespace PXTEngine {

	class Material : public Resource {
	public:
	protected:
		Shared<Image> m_albedo = nullptr;
		Shared<Image> m_normalMap = nullptr;
		Shared<Image> m_metalnessMap = nullptr;
		Shared<Image> m_roughnessMap = nullptr;
		Shared<Image> m_ambientOcclusionMap = nullptr;

	};
}