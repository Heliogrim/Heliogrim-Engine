#pragma once
#include "Actor.hpp"
#include "StaticGeometryComponent.hpp"

namespace hg {
    class StaticGeometryActor :
        public Actor {
    public:
        using this_type = StaticGeometryActor;
        using underlying_type = Actor;

    public:
        StaticGeometryActor(cref<ActorInitializer> initializer_);

    private:
        ptr<StaticGeometryComponent> _staticGeometryComponent;

    public:
        [[nodiscard]] ptr<StaticGeometryComponent> getStaticGeometryComponent() const noexcept;
    };
}
