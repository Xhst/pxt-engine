
#pragma once

#include "core/uuid.hpp"

namespace CGEngine
{
    struct IDComponent {
        UUID uuid;

        IDComponent() = default;
        IDComponent(const IDComponent&) = default;
    };

    struct NameComponent {
        std::string Name;

        NameComponent() = default;
        NameComponent(const NameComponent&) = default;
        NameComponent(const std::string& name) : Name(name) {}
    };
}