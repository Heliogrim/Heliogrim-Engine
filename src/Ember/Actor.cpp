#include "Actor.hpp"

#include <Engine.ACS/Registry.hpp>
#include <Engine.Common/Concurrent/Promise.hpp>
#include <Engine.Session/Session.hpp>

#include "IComponentRegisterContext.hpp"

using namespace ember;

Future<ptr<Actor>> ember::CreateActor() noexcept {

    concurrent::promise<ptr<Actor>> p {
        _STD move([]() {

            auto* registry = engine::Session::get()->modules().acsRegistry();
            auto* actor = registry->createActor();

            return actor;
        })
    };

    auto f = p.get();
    p();

    return Future { _STD move(f) };
}

Future<ptr<Actor>> ember::CreateActor(cref<ActorClass> class_) noexcept {
    throw NotImplementedException();
}

Future<ptr<Actor>> ember::CreateActor(cref<ActorClass> class_, const ptr<SerializedActor> serialized_) noexcept {
    throw NotImplementedException();
}

bool ember::MountActor(const ptr<Actor> actor_) noexcept {
    throw NotImplementedException();
}

Future<ptr<Actor>> ember::CloneActor(const ptr<Actor> actor_) noexcept {
    throw NotImplementedException();
}

Future<ptr<Actor>> ember::SpawnActor(cref<ActorClass> class_) noexcept {
    throw NotImplementedException();
}

Future<ptr<Actor>> ember::SpawnActor(cref<ActorClass> class_, const ptr<SerializedActor> serialized_) noexcept {
    throw NotImplementedException();
}

Future<bool> ember::Destroy(mref<ptr<Actor>> actor_) noexcept {
    throw NotImplementedException();
}

Actor::Actor() noexcept = default;

Actor::~Actor() noexcept {}

actor_guid Actor::guid() const noexcept {
    throw NotImplementedException();
}

ptr<ActorComponent> Actor::getRootComponent() const noexcept {
    return _rootComponent;
}

cref<Transform> Actor::getWorldTransform() const noexcept {

    if (_rootComponent != nullptr) {
        return _rootComponent->getWorldTransform();
    }

    return Transform {};
}

void Actor::registerComponents(const ptr<IComponentRegisterContext> context_) {

    /**
     * Guard for non rooted actor
     */
    if (_rootComponent == nullptr) {
        return;
    }

    /**
     *
     */
    context_->add(_rootComponent);
}
