#include "Entity.hpp"

#include <Engine.ECS/Entity.hpp>
#include <Engine.ECS/Registry.hpp>
#include <Engine.ECS.Subsystem/Components/TransformComponent.hpp>
#include <Engine.Session/Session.hpp>

#include "Engine.ECS.Subsystem/Subsystem.hpp"
#include "Engine.ECS/System.hpp"

using namespace ember;

bool ember::Valid(cref<Entity> entity_) noexcept {
    if (entity_.guid() == invalid_entity_guid) {
        return false;
    }

    return true;
}

future<Entity> ember::CreateEntity() noexcept {

    // TODO: Replace ~ Temporary
    auto e { engine::ecs::Entity::from(generate_entity_guid()) };

    /**
     *
     */
    static_cast<engine::ecs::DynamicEntity&>(e).record<engine::ecs::subsystem::TransformComponent>();

    /**
     *
     */
    auto fs = make_sptr<concurrent::future_state<Entity>>();
    concurrent::future<Entity> f { fs };

    fs->set(_STD move(e));
    return { _STD move(f) };
}

future<Entity> ember::CloneEntity(cref<Entity> entity_) noexcept {
    throw NotImplementedException {};
}

future<bool> ember::Destroy(mref<Entity> entity_) noexcept {
    throw NotImplementedException {};
}

EntityBase::EntityBase(cref<entity_guid> guid_) noexcept :
    _guid(guid_) {}

entity_guid EntityBase::guid() const noexcept {
    return _guid;
}

ptr<void> EntityBase::get(cref<component_type_id> componentTypeId_) const noexcept {
    return engine::Session::get()->ecs()->getComponent(componentTypeId_, _guid);
}

bool EntityBase::has(cref<component_type_id> componentTypeId_) const noexcept {
    return engine::Session::get()->ecs()->getComponent(componentTypeId_, _guid) != nullptr;
}

void EntityBase::record(cref<component_type_id> componentTypeId_) noexcept {
    return engine::Session::get()->ecs()->makeComponent(componentTypeId_, _guid);
}

Entity::Entity() noexcept :
    EntityBase() {}

Entity::Entity(cref<Entity> other_) noexcept = default;

Entity::Entity(mref<Entity> other_) noexcept = default;

Entity::Entity(cref<entity_guid> guid_) noexcept :
    EntityBase(guid_) {}

ref<Entity> Entity::operator=(cref<Entity> other_) noexcept {
    if (_STD addressof(other_) != this) {
        _guid = other_._guid;
    }
    return *this;
}

ref<Entity> Entity::operator=(mref<Entity> other_) noexcept {
    if (_STD addressof(other_) != this) {
        _guid = other_._guid;
    }
    return *this;
}

cref<math::Transformation> Entity::transform() const noexcept {
    #ifdef _DEBUG
    auto comp = get(engine::ecs::subsystem::TransformComponent::type_id);
    DEBUG_ASSERT(comp != nullptr, "")
    return static_cast<ptr<engine::ecs::subsystem::TransformComponent>>(comp)->transformation();
    #else
    return static_cast<ptr<engine::ecs::subsystem::TransformComponent>>(
        get(engine::ecs::subsystem::TransformComponent::type_id)
    )->transformation();
    #endif
}

void Entity::setTransform(cref<math::Transformation> transformation_) {
    /**
     * Get Component
     */
    auto* component = get(engine::ecs::subsystem::TransformComponent::type_id);

    /**
     * Update Component
     */
    const_cast<math::Transformation&>(
        static_cast<ptr<engine::ecs::subsystem::TransformComponent>>(component)->transformation()
    ) = transformation_;

    /**
     * Signal Staged
     */
    engine::Session::get()->subsystem()->signalUpdate(_guid, engine::ecs::subsystem::TransformComponent::type_id,
        component);
}
