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

ptr<Actor> Registry::createActor(const ptr<const ActorClass> actorClass_, cref<ActorInitializer> initializer_) {

    const auto guid = generate_actor_guid();

    /**/

    auto* const pool { _actorPools.at(actorClass_->typeId()) };
    pool->insert(guid, initializer_);

    /**/

    auto* actor { static_cast<ptr<Actor>>(pool->get(guid)) };
    actor->unsafe_set_guid(guid);

    // Warning: Error prone behaviour
    // Warning: Just a hotfix
    // We can expect the have the class pointer at the base address, due to the fact
    //  that Actor is based on EmberObject
    static_assert(sizeof(EmberObject) == sizeof(ptr<EmberClass>));
    const auto** classLocator = reinterpret_cast<ptr<ptr<const ActorClass>>>(
        _STD addressof(*static_cast<ptr<EmberObject>>(actor))
    );
    *classLocator = actorClass_;

    /**/

    return actor;
}

ptr<Actor> Registry::createActor(const ptr<const ActorClass> actorClass_, cref<ActorInitializer> initializer_,
    std::nothrow_t) noexcept {

    const auto guid = generate_actor_guid();

    const auto poolIter { _actorPools.find(actorClass_->typeId()) };
    if (poolIter == _actorPools.end()) {
        return nullptr;
    }

    /**/

    auto* const pool { _actorPools.at(actorClass_->typeId()) };
    pool->insert(guid, initializer_);

    /**/

    auto* actor { static_cast<ptr<Actor>>(pool->get(guid)) };
    if (actor == nullptr) {
        return nullptr;
    }

    /**/

    actor->unsafe_set_guid(guid);

    // Warning: Error prone behaviour
    // Warning: Just a hotfix
    // We can expect the have the class pointer at the base address, due to the fact
    //  that Actor is based on EmberObject
    static_assert(sizeof(EmberObject) == sizeof(ptr<EmberClass>));
    const auto** classLocator = reinterpret_cast<ptr<ptr<const ActorClass>>>(
        _STD addressof(*static_cast<ptr<EmberObject>>(actor))
    );
    *classLocator = actorClass_;

    /**/

    return actor;
}

ptr<Actor> Registry::createActor(cref<ActorInitializer> initializer_) noexcept {

    const auto guid = generate_actor_guid();

    /**/

    auto* const pool = getOrCreateActorPool<Actor>();
    pool->emplace(guid, initializer_);

    /**/

    auto* actor { pool->get(guid) };
    if (actor == nullptr) {
        return nullptr;
    }

    /**/

    actor->unsafe_set_guid(guid);

    // Warning: Error prone behaviour
    // Warning: Just a hotfix
    // We can expect the have the class pointer at the base address, due to the fact
    //  that Actor is based on EmberObject
    static_assert(sizeof(EmberObject) == sizeof(ptr<EmberClass>));
    const auto** classLocator = reinterpret_cast<ptr<ptr<const ActorClass>>>(
        _STD addressof(*static_cast<ptr<EmberObject>>(actor))
    );
    *classLocator = EmberClass::of<Actor>();

    /**/

    return actor;
}

void Registry::destroyActor(mref<ptr<Actor>> actor_) {
    auto* const pool { _actorPools.at(actor_->getClass()->typeId()) };
    pool->erase(actor_->guid());
}
