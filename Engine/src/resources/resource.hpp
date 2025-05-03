#pragma once

#include "core/uuid.hpp"

#include <string>

namespace PXTEngine {

	using ResourceId = UUID;

	struct ResourceInfo
	{
		virtual ~ResourceInfo() = default;
	};

	/**
	 * @class Resource
	 *
	 * @brief Base class for all resources in the engine.
	 */
	class Resource {
	public:
		enum class Type : uint8_t {
			Image,
			Model,
			Mesh,
			Material,
		};

		Resource() = default;
		virtual ~Resource() = default;

		virtual Type getType() const = 0;

		// The default constructor is called and the ID is set to a new UUID.
		ResourceId id;
	};

}