#pragma once

#include <string>

namespace CGEngine {

    enum UUIDVersion {
        V4,
        V7
    };

    class UUID {
    public:
        UUID();
        UUID(UUIDVersion version);
        UUID(const std::string& uuid);
        UUID(const UUID&) = default;

        operator size_t() const {
            return std::hash<std::string>{}(m_uuid);
        }

        std::string toString() const {
            return m_uuid;
        }


    private:
        std::string m_uuid;

        static std::string generateUUIDv4();
        static std::string generateUUIDv7();
        
    };
}

namespace std {

	template <typename T> struct hash;

	template<>
	struct hash<CGEngine::UUID>
	{
		std::size_t operator()(const CGEngine::UUID& uuid) const
		{
			return (size_t) uuid;
		}
	};

}