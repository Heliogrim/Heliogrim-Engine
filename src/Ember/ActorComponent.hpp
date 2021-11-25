#pragma once
#include "LogicComponent.hpp"

namespace ember {

    /**
     * Forward Declaration
     */
    class Actor;

    class ActorComponent :
        public LogicComponent {
    public:
        using this_type = ActorComponent;
        using underlying_type = LogicComponent;

    public:
        [[nodiscard]] ptr<Actor> getRootActor() const noexcept;

        [[nodiscard]] ptr<ActorComponent> getRootComponent() const noexcept;

        [[nodiscard]] ptr<ActorComponent> getParentComponent() const noexcept;

    public:
        [[nodiscard]] cref<math::Bounding> getBoundaries() const noexcept;

        [[nodiscard]] cref<math::Transformation> getWorldTransform() const noexcept;

        [[nodiscard]] cref<math::Transformation> getLocalTransform() const noexcept;
    };

}
