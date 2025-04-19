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
        UUID();
        UUID(UUIDVersion version);
        UUID(const std::string& uuid);
        UUID(const UUID&) = default;

        /**
         * @brief Converts the UUID object to a hashable size_t.
         *
         * This operator allows the UUID to be used in hash-based containers
         * like std::unordered_map or std::unordered_set.
         *
         * @return The hash of the UUID string.
         */
        operator size_t() const {
            return std::hash<std::string>{}(m_uuid);
        }

        /**
         * @brief Converts the UUID to its string representation.
         *
         * @return A string representing the UUID.
         */
        std::string toString() const {
            return m_uuid;
        }

    private:
        std::string m_uuid;

        /**
         * @brief Generates a random UUID (Version 4).
         *
         * @return A string containing the generated UUID (V4).
         */
        static std::string generateUUIDv4();

        /**
         * @brief Generates a time-based UUID (Version 7).
         *
         * @return A string containing the generated UUID (V7).
         */
        static std::string generateUUIDv7();

    };
}

namespace std {

    /**
     * @brief Specialization of std::hash for PXTEngine::UUID.
     *
     * This allows UUIDs to be used as keys in unordered containers,
     * such as std::unordered_map and std::unordered_set.
     */
    template<>
    struct hash<PXTEngine::UUID>
    {
        /**
         * @brief Computes the hash for a given UUID.
         *
         * @param uuid The UUID to be hashed.
         * @return The hash value of the UUID as size_t.
         */
        std::size_t operator()(const PXTEngine::UUID& uuid) const
        {
            return (size_t)uuid;
        }
    };

}
