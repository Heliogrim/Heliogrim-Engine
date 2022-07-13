#include "Registry.hpp"

#include <Ember/Actor.hpp>

using namespace ember::engine::acs;
using namespace ember;

Registry::~Registry() = default;

void Registry::releaseActorComponent(cref<actor_guid> guid_, cref<type_id> typeId_) {
    auto* entry { _pools.at(typeId_) };
    auto* pool { static_cast<ptr<PoolWrapperBase>>(entry) };

    pool->erase(guid_);
}

ptr<Actor> Registry::createActor() noexcept {

    const auto guid = generate_actor_guid();

    _defaultActorPool->insert(guid, Actor {});
    auto* actor { _defaultActorPool->get(guid) };
    actor->unsafe_set_guid(guid);
    return actor;
}

void Registry::destroyActor(mref<ptr<Actor>> actor_) {
    _defaultActorPool->erase(actor_->guid());
}
