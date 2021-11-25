#include "Actor.hpp"

#include <Engine.Session/Session.hpp>
#include <Engine.ACS/Registry.hpp>

#include "Engine.Common/Concurrent/Promise.hpp"

using namespace ember;

future<ptr<Actor>> ember::CreateActor() noexcept {

    concurrent::promise<ptr<Actor>> p {
        _STD move([]() {

            auto* registry = engine::Session::get()->registry();
            auto* actor = registry->createActor();

            return actor;
        })
    };

    auto f = p.get();
    p();

    return future { _STD move(f) };
}

future<ptr<Actor>> ember::CreateActor(cref<ActorClass> class_) noexcept {
    throw NotImplementedException();
}

bool ember::MountActor(const ptr<Actor> actor_) noexcept {
    throw NotImplementedException();
}

future<ptr<Actor>> ember::CloneActor(const ptr<Actor> actor_) noexcept {
    throw NotImplementedException();
}

future<ptr<Actor>> ember::SpawnActor(cref<ActorClass> class_) noexcept {
    throw NotImplementedException();
}

future<ptr<Actor>> ember::SpawnActor(cref<ActorClass> class_, const ptr<SerializedActor> serialized_) noexcept {
    throw NotImplementedException();
}

future<bool> ember::Destroy(mref<ptr<Actor>> actor_) noexcept {
    throw NotImplementedException();
}

Actor::Actor(const ptr<ActorInitializer> initializer_) noexcept {}

Actor::~Actor() noexcept {}

actor_guid Actor::guid() const noexcept {
    throw NotImplementedException();
}

ptr<ActorComponent> Actor::getRootComponent() const noexcept {
    return _rootComponent;
}

cref<math::Transformation> Actor::getWorldTransform() const noexcept {

    if (_rootComponent != nullptr) {
        return _rootComponent->getWorldTransform();
    }

    return math::ZeroTransformation();
}
