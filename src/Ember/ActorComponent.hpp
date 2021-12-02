#pragma once
#include "CachedActorPointer.hpp"
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
        ActorComponent(mref<CachedActorPointer> owner_, const ptr<ActorComponent> parent_);

    private:
        CachedActorPointer _owner;

    public:
        [[nodiscard]] ptr<Actor> getOwner() const noexcept;

    private:
        ptr<ActorComponent> _parent;

    public:
        [[nodiscard]] ptr<Actor> getRootActor() const noexcept;

        [[nodiscard]] ptr<ActorComponent> getRootComponent() const noexcept;

        [[nodiscard]] ptr<ActorComponent> getParentComponent() const noexcept;

    private:
        math::Bounding _boundaries;

    public:
        [[nodiscard]] cref<math::Bounding> getBoundaries() const noexcept;

    private:
        Vector3 _localPosition;
        Quatern _localRotation;
        Vector3 _localScale;

    public:
        [[nodiscard]] Transform getLocalTransform() const noexcept;

    private:
        Transform _worldTransform;

    public:
        [[nodiscard]] cref<Transform> getWorldTransform() const noexcept;
    };

}
