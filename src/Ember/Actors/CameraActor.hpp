#pragma once

#include "../Actor.hpp"
#include "../Components/CameraComponent.hpp"

namespace ember {
    class CameraActor :
        public Actor {
    public:
        using this_type = CameraActor;
        using underlying_type = Actor;

        inline constexpr static type_id typeId { "CameraActor"_typeId };

    public:
        CameraActor(cref<ActorInitializer> initializer_);

    private:
        ptr<CameraComponent> _cameraComponent;
        ptr<ActorComponent> _mountComponent;

    public:
        [[nodiscard]] const ptr<CameraComponent> getCameraComponent() const noexcept;
    };
}
