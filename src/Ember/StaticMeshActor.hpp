#pragma once
#include "Actor.hpp"
#include "StaticMeshComponent.hpp"

namespace ember {

    class StaticMeshActor :
        public Actor {
    public:
        using this_type = StaticMeshActor;
        using underlying_type = Actor;

    public:
        StaticMeshActor();

    private:
        ptr<StaticMeshComponent> _staticMeshComponent;

    public:
        [[nodiscard]] ptr<StaticMeshComponent> getStaticMeshComponent() const noexcept;
    };
}
