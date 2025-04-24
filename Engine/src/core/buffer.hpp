#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <cstring>

namespace PXTEngine {

	class Buffer {
	public:
		Buffer() = default;

		Buffer(size_t size) : m_size(size) {
			m_bytes = (uint8_t*) malloc(size);
		}

		Buffer(const void* data, const size_t size) : m_bytes((uint8_t*) data), m_size(size) {}

		Buffer(const Buffer&) = default;

		operator bool() const {
			return m_bytes != nullptr;
		}

		static Buffer copy(Buffer& other) {
			Buffer copy(other.m_size);
			memcpy(copy.m_bytes, other.m_bytes, other.m_size);

			return copy;
		}

		void release() {
			free(m_bytes);
			m_bytes = nullptr;
			m_size = 0;
		}

		template<typename T>
		T* get() {
			return (T*) m_bytes;
		}


	private:
		uint8_t* m_bytes = nullptr;
		size_t m_size = 0;
	};
}