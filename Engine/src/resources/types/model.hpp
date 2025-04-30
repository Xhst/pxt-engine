#pragma once

#include "resources/resource.hpp"
#include "resources/types/image.hpp"
#include "resources/types/mesh.hpp"
#include "resources/types/material.hpp"

#include <vector>

namespace PXTEngine {

    class Model : public Resource {
    public:
		virtual const std::vector<Shared<Mesh>>& getMeshes() const = 0;
		virtual const std::vector<Shared<Image>>& getTextures() const = 0;
		virtual const std::vector<Shared<Material>>& getMaterials() const = 0;

		static Type getStaticType() { return Type::Model; }
    };
}