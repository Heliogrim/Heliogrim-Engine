#include "Entity.hpp"

using namespace ember::engine::ecs;
using namespace ember;

const entity_guid& Entity::get_guid() const {
    return _guid;
}

entity_guid Entity::get_guid() {
    return _guid;
}

Entity::Entity() :
    _guid(invalid_entity_guid) {}

Entity::Entity(const entity_guid& entityId_) :
    _guid(entityId_) {}

DynamicEntity::DynamicEntity(const entity_guid& entityId_) :
    Entity(entityId_) {}
