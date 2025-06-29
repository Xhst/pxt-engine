#pragma once

#include "core/pch.hpp"
#include "core/uuid.hpp"


namespace PXTEngine {

	using ResourceId = UUID;

	/**
	 * @struct ResourceInfo
	 *
	 * @brief Base class for resource information.
	 * It's used to store additional information about resources.
	 */
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