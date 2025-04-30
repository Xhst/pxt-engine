#pragma once

#include <string>

namespace PXTEngine {

    /**
     * @brief Enumeration representing the versions of UUID that can be generated.
     *
     * V4: Randomly generated UUID (UUID version 4).
     * V7: Time-based UUID (UUID version 7).
     */
    enum UUIDVersion {
        V4,
        V7
    };

    /**
     * @brief Class representing a Universally Unique Identifier (UUID).
     */
    class UUID {
    public:

	    /**
		 * @brief Constructs a new UUID of version 7 (time-based).
	     */
	    UUID();

        /**
		 * @brief Constructs a UUID of the specified version.
		 * 
		 * @param version The version of the UUID to generate.
         */
        explicit UUID(UUIDVersion version);

        /**
	     * @brief Constructs a UUID from a standard hyphenated string representation.
	     * Parses a string in the format "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx".
	     * If the string is not valid, the UUID will be initialized to zero.
	     *
	     * @param uuidString The string representation of the UUID.
	     */
        explicit UUID(const std::string& uuidString);

		bool operator==(const UUID& other) const {
			return m_high == other.m_high && m_low == other.m_low;
		}

        /**
         * @brief Converts the UUID's binary representation into the standard
         * hyphenated string format: "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx".
         * 
         * @return A std::string representing the UUID.
         */
        [[nodiscard]] std::string toString() const;

    private:
		UUID(const uint64_t high, const uint64_t low)
			: m_high(high), m_low(low) {}

		uint64_t m_high = 0;  // High 64 bits of the UUID
		uint64_t m_low = 0;   // Low 64 bits of the UUID

        /**
	     * @brief Generates a Universally Unique Identifier (UUID) according to version 4.
	     * UUID v4 is random and not time-based. Thread-safe.
	     * 
	     * @return A new UUID of version 4 (binary representation).
	     */
        static UUID generateUUIDv4();

        /**
	     * @brief Generates a Universally Unique Identifier (UUID) according to version 7.
	     * UUID v7 is time-based with a random component.
	     *
	     * @note Deviation from Standard UUIDv7 (RFC 9562): This implementation omits the
	     * standard monotonic counter. The 12 bits intended for the counter are filled
	     * with random data instead, losing the standard guarantee of monotonic ordering
	     * within the same millisecond and relies solely on randomness for uniqueness in
		 * rapid bursts within that time. (Very unlikely to happen in practice)
	     *
	     * @return A new UUID of version 7 (binary representation).
	     */
        static UUID generateUUIDv7();

        friend struct std::hash<PXTEngine::UUID>;
    };
}

/**
 * @brief Specialization of std::hash for PXTEngine::UUID.
 *
 * This allows UUIDs to be used as keys in unordered containers,
 * such as std::unordered_map and std::unordered_set.
 */
template<>
struct std::hash<PXTEngine::UUID> {
    /**
     * @brief Computes the hash for a given UUID.
     * Combines the hash values of the high and low 64-bit components.
     *
     * @param uuid The UUID to be hashed.
     * @return The hash value of the UUID as size_t.
     */
    std::size_t operator()(const PXTEngine::UUID& uuid) const noexcept {
        constexpr std::hash<uint64_t> hasher;

        const uint64_t h1 = hasher(uuid.m_high);
        const uint64_t h2 = hasher(uuid.m_low);

        return h1 ^ h2;
    }
};
