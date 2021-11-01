#pragma once

#include <Engine.Common/Math/Transformation.hpp>
#include <Engine.ECS/ComponentTypeId.hpp>

namespace ember::engine::ecs::subsystem {

    class TransformComponent {
    public:
        constexpr static component_type_id type_id { "TransformComponent"_typeId };

    public:
        TransformComponent() noexcept = default;

    private:
        math::Transformation _transform;

    public:
        [[nodiscard]] cref<math::Transformation> transformation() const noexcept;
    };

}
