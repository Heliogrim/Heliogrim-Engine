#pragma once

#include <Engine.Reflect/HeliogrimReflect.hpp>
#include <Engine.ACS/ComponentTypeId.hpp>

namespace hg {
    class LogicComponent :
        public HeliogrimObject {
    public:
        using this_type = LogicComponent;
        using underlying_type = HeliogrimObject;

    public:
        LogicComponent(mref<component_type_id> typeId_) noexcept;

        ~LogicComponent() = default;

    private:
        component_type_id _typeId;

    public:
        [[nodiscard]] cref<component_type_id> getTypeId() const noexcept;
    };
}
