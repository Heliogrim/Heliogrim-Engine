#include "Actor.hpp"

#include <Engine.ACS/Registry.hpp>
#include <Engine.Common/Concurrent/Promise.hpp>
#include <Engine.Session/Session.hpp>

#include "IComponentRegisterContext.hpp"

using namespace ember;

Actor::Actor() noexcept = default;

Actor::~Actor() noexcept = default;

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

cref<CompactSet<ptr<ActorComponent>>> Actor::getComponents() const noexcept {
    return _components;
}

void Actor::addComponent(const ptr<ActorComponent> component_) {

    /**
     *
     */
    if (component_->getParentComponent() == nullptr && !_rootComponent) {
        _rootComponent = component_;
    }

    #ifdef _DEBUG
    if (component_->getParentComponent() != nullptr) {
        assert(
            _components.contains(component_->getParentComponent()) &&
            "Failed to ensure acknowledged parent component.");
    }
    #endif

    _components.insert(component_);
}

void Actor::registerComponents(const ptr<IComponentRegisterContext> context_) {

    /**
     * Guard for non rooted actor
     */
    if (_rootComponent == nullptr) {
        return;
    }

    /**
     * Insert components ordered by hierarchy from parent to child (copy components)
     */
    auto components { getComponents() };

    auto iter { components.begin() };
    while (!components.empty()) {

        /**
         *
         */
        const auto entry { *iter };
        const auto parent { entry->getParentComponent() };

        /**
         *
         */
        auto prevIter = components.find(parent);
        if (prevIter != components.end()) {
            iter = _STD move(prevIter);
            continue;
        }

        /**
         * Insert component
         */
        context_->add(entry);

        /**
         *
         */
        auto nextIter { components.erase(iter) };
        iter = _STD move(nextIter != components.end() ? nextIter : components.begin());
    }
}

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
