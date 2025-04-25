#pragma once

#include "core/uuid.hpp"

#include <string>
#include <utility>

namespace PXTEngine {

	using ResourceId = std::string;

	class Resource {
	public:
		enum class Type : uint8_t {
			Image,
			Model,
		};

    Resource(const ResourceId& id, const Type type) : m_id(id), m_type(type) {}

	ResourceId getId() const {
		return m_id;
	}

	Type getType() const {
		return m_type;
	}

	private:
		ResourceId m_id{};
		Type m_type;
	};

}