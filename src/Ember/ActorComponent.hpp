#pragma once
#include "CachedActorPointer.hpp"
#include "LogicComponent.hpp"
#include "Inbuilt.hpp"

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

        inline constexpr static component_type_id typeId { "ActorComponent"_typeId };

    public:
        ActorComponent(mref<component_type_id> typeId_, mref<CachedActorPointer> owner_,
            mref<ptr<ActorComponent>> parent_);

        ActorComponent(mref<CachedActorPointer> owner_, mref<ptr<ActorComponent>> parent_);

        ActorComponent(mref<ActorComponent> other_) noexcept = default;

        ActorComponent(cref<ActorComponent>) = delete;

        ~ActorComponent() = default;

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
        Transform _localTransform;

    public:
        [[nodiscard]] cref<Transform> getLocalTransform() const noexcept;

        [[nodiscard]] ref<Transform> getLocalTransform() noexcept;

    private:
        Transform _worldTransform;

    public:
        [[nodiscard]] cref<Transform> getWorldTransform() const noexcept;
    };
}
