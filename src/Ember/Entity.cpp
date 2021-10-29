#include "Entity.hpp"

#include <Engine.ECS/Entity.hpp>
#include <Engine.ECS/Registry.hpp>

using namespace ember;

engine::ecs::entity_guid EntityBase::guid() const noexcept {
    return _guid;
}

ptr<void> EntityBase::get(cref<component_type_id> componentTypeId_) const noexcept {
    return engine::ecs::registry::get().get_pool_adapter(componentTypeId_)->get(_guid);
}

bool EntityBase::has(cref<component_type_id> componentTypeId_) const noexcept {
    return engine::ecs::registry::get().get_pool_adapter(componentTypeId_)->get(_guid) != nullptr;
}

void EntityBase::record(cref<component_type_id> componentTypeId_) noexcept {
    engine::ecs::registry::get().get_pool_adapter(componentTypeId_)->insert(_guid);
}

Entity::Entity() noexcept :
    EntityBase() {}

Entity::Entity(cref<Entity> other_) noexcept = default;

Entity::Entity(mref<Entity> other_) noexcept = default;

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

bool entity::valid(cref<Entity> entity_) noexcept {
    if (entity_.guid() == engine::ecs::invalid_entity_guid) {
        return false;
    }

    return true;
}

future<Entity> entity::create() noexcept {

    // TODO: Replace ~ Temporary
    auto e { engine::ecs::Entity::from(engine::ecs::generate_entity_guid()) };

    auto fs = make_sptr<concurrent::future_state<Entity>>();
    concurrent::future<Entity> f { fs };

    fs->set(_STD move(e));
    return { _STD move(f) };
}

future<Entity> entity::clone(cref<Entity> entity_) noexcept {
    throw NotImplementedException {};
}

future<bool> entity::destroy(mref<Entity> entity_) noexcept {
    throw NotImplementedException {};
}
