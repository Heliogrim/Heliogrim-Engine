#pragma once

#include <Engine.ACS/ComponentTypeId.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>

namespace hg {
    class LogicComponent :
        public InheritBase<LogicComponent> {
    public:
        using this_type = LogicComponent;
        using underlying_type = InheritBase<LogicComponent>;

    public:
        LogicComponent(mref<component_type_id> typeId_) noexcept;

        ~LogicComponent() = default;

    private:
        component_type_id _typeId;

    public:
        [[nodiscard]] cref<component_type_id> getTypeId() const noexcept;
    };
}
