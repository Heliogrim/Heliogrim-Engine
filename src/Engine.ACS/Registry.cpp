#include "Registry.hpp"

#include <Ember/Actor.hpp>

using namespace ember::engine::acs;
using namespace ember;

Registry::~Registry() = default;

ptr<Actor> Registry::createActor() noexcept {

    const auto guid = generate_actor_guid();

    _defaultActorPool->insert(guid, Actor {});
    auto* actor { _defaultActorPool->get(guid) };
    actor->unsafe_set_guid(guid);
    return actor;
}
