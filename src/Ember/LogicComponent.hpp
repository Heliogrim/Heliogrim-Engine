#pragma once

#include "Inbuilt.hpp"
#include "Scene.hpp"

namespace ember {

    class LogicComponent :
        public EmberObject {
    public:
        using this_type = LogicComponent;
        using underlying_type = EmberObject;

    public:
        LogicComponent(mref<component_type_id> typeId_) noexcept;

        ~LogicComponent() = default;

    private:
        component_type_id _typeId;

    public:
        [[nodiscard]] cref<component_type_id> getTypeId() const noexcept;
    };
}
