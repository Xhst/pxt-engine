#include "core/uuid.hpp"

#include <random>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace PXTEngine {


	UUID::UUID(const std::string& uuid) : m_uuid(uuid) {}
	UUID::UUID() : m_uuid(generateUUIDv7()) {}
	UUID::UUID(UUIDVersion version) {
		switch (version) {
		case UUIDVersion::V4:
			m_uuid = generateUUIDv4();
			break;
		case UUIDVersion::V7:
			m_uuid = generateUUIDv7();
			break;
		}
	}

	std::string UUID::generateUUIDv4() {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);

		std::stringstream ss;
		ss << std::hex << std::setfill('0');
		ss << std::setw(8) << dist(gen) << "-";
		ss << std::setw(4) << (dist(gen) & 0xFFFF) << "-";
		ss << std::setw(4) << ((dist(gen) & 0x0FFF) | 0x4000) << "-"; // 4 indicates version 4
		ss << std::setw(4) << ((dist(gen) & 0x3FFF) | 0x8000) << "-"; // 8 indicates variant
		ss << std::setw(12) << dist(gen) << dist(gen);

		return ss.str();
	}

	// Static method to generate a UUIDv7 (time-based)
	std::string UUID::generateUUIDv7() {
		auto now = std::chrono::system_clock::now();
		auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<uint16_t> dist16(0, 0xFFFF);
		std::uniform_int_distribution<uint32_t> dist32(0, 0xFFFFFFFF);

		std::stringstream ss;
		ss << std::hex << std::setfill('0');

		// Time component (millis)
		ss << std::setw(8) << (millis >> 16) << "-";  // high part of timestamp (32 bits)
		ss << std::setw(4) << (millis & 0xFFFF) << "-";  // low part of timestamp (16 bits)

		// Random part
		ss << std::setw(4) << ((dist16(gen) & 0x0FFF) | 0x7000) << "-"; // 7 indicates version 7
		ss << std::setw(4) << ((dist16(gen) & 0x3FFF) | 0x8000) << "-"; // 8 indicates variant
		ss << std::setw(12) << dist32(gen);  // random 48 bits

		return ss.str();
	}
};
