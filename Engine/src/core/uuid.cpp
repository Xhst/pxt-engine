#include "core/uuid.hpp"

#include "core/pch.hpp"

namespace PXTEngine {

	UUID::UUID(const std::string& uuidString) {
        // Length: 32 hex characters + 4 hyphens = 36 characters.
        if (uuidString.length() != 36) return;
        

        // Check hyphen positions.
        if (uuidString[8]  != '-' || uuidString[13] != '-' || 
            uuidString[18] != '-' || uuidString[23] != '-') {
            return;
        }

        // Create a copy of the string and remove the hyphens.
        std::string hexString = uuidString;
        std::erase(hexString, '-');

        try {
            // The first 16 hex characters represent the high 64 bits.
            m_high = std::stoull(hexString.substr(0, 16), nullptr, 16);
            // The next 16 hex characters represent the low 64 bits.
            m_low = std::stoull(hexString.substr(16, 16), nullptr, 16);
        } catch (const std::exception& _) {
            return;
        }
    }

	UUID::UUID() {
		*this = generateUUIDv7();
	}

	UUID::UUID(const UUIDVersion version) {
		switch (version) {
		case V4:
			*this = generateUUIDv4();
			break;
		case V7:
			*this = generateUUIDv7();
			break;
		}
	}

	UUID UUID::generateUUIDv4() {
		// UUID v4 structure: 128 bits of random data with version and variant bits set.
		// Version 4 (0100) is in bits 60-63. Variant (10xx) is in bits 64-65.

		std::random_device rd;
		std::mt19937_64 gen(rd());
		std::uniform_int_distribution<uint64_t> dist;

		uint64_t randomHigh = dist(gen);
		uint64_t randomLow = dist(gen);

		// Set version 4 (0100) in high part (bits 12-15 from right).
		randomHigh = (randomHigh & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;

		// Set variant 1 (10xx) in low part (bits 62-63 from right).
		randomLow = (randomLow & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

        return { randomHigh, randomLow };
	}

    UUID UUID::generateUUIDv7() {
        // UUID v7 structure: 48-bit timestamp | 4-bit version |
        //                    | 12-bit rand_a | 2-bit variant | 62-bit rand_b

        // Get 48-bit Unix Epoch timestamp in milliseconds.
        auto now = std::chrono::system_clock::now();
        uint64_t timestampMs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        timestampMs &= 0xFFFFFFFFFFFFULL; // Mask to 48 bits

        // Generate random bits for rand_a (12 bits) and rand_b (62 bits).
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dist;

        const uint64_t randomAPart = dist(gen); // Use lower 12 bits for rand_a
        const uint64_t randomBPart = dist(gen); // Use lower 62 bits for rand_b

        // Assemble high 64 bits: timestamp (48) | version (7) | rand_a (12 random)
        uint64_t high = (timestampMs << 16) |   // Timestamp shifted
			(0x7ULL << 12) |                    // Version 7 shifted
			(randomAPart & 0xFFFULL);           // rand_a (12 random bits) placed

        // Assemble low 64 bits: variant (10xx) | rand_b (62 random)
			uint64_t low = (0x8ULL << 60) |       // Variant 1 (10) shifted
            (randomBPart & 0x3FFFFFFFFFFFFFFULL); // Rand_b (62 random bits) placed

        return { high, low };
    }

    std::string UUID::toString() const {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');

        // Extract and format parts from m_high and m_low based on standard UUID string layout.
        ss << std::setw(8) << (m_high >> 32) << "-";            // First 32 bits of m_high
        ss << std::setw(4) << ((m_high >> 16) & 0xFFFF) << "-"; // Next 16 bits of m_high
        ss << std::setw(4) << (m_high & 0xFFFF) << "-";         // Last 16 bits of m_high
        ss << std::setw(4) << (m_low >> 48) << "-";             // First 16 bits of m_low
        ss << std::setw(12) << (m_low & 0xFFFFFFFFFFFFULL);     // Last 48 bits of m_low

        return ss.str();
    }

};
