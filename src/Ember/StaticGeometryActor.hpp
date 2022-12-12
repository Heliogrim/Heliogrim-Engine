#pragma once
#include "Actor.hpp"
#include "StaticGeometryComponent.hpp"

namespace ember {
    class StaticGeometryActor :
        public Actor {
    public:
        using this_type = StaticGeometryActor;
        using underlying_type = Actor;

    public:
        StaticGeometryActor();

        StaticGeometryActor(cref<ActorInitializer> initializer_);

    private:
        ptr<StaticGeometryComponent> _staticGeometryComponent;

    public:
        [[nodiscard]] ptr<StaticGeometryComponent> getStaticGeometryComponent() const noexcept;
    };
}
