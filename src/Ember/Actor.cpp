#include "Actor.hpp"

#include <Engine.ACS/Registry.hpp>
#include <Engine.Common/Concurrent/Promise.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Session.hpp>
#include <Engine.Core/SessionState.hpp>
#include <Engine.Core/WorldContext.hpp>

#include "IComponentRegisterContext.hpp"

#include "ActorComponent.hpp"
#include "Session.hpp"
#include "World.hpp"

using namespace ember;

Actor::Actor() noexcept :
    _guid(invalid_actor_guid),
    _rootComponent(nullptr),
    _components() {}

Actor::Actor(mref<Actor> other_) noexcept :
    _guid(_STD move(other_._guid)),
    _rootComponent(_STD move(other_._rootComponent)),
    _components(_STD move(other_._components)) {
    assert(_guid == invalid_actor_guid && other_._guid == invalid_actor_guid);
    assert(_rootComponent == other_._rootComponent && _rootComponent == nullptr);
    assert(_components.empty() && other_._components.empty());
}

Actor::~Actor() noexcept = default;

actor_guid Actor::guid() const noexcept {
    return _guid;
}

void Actor::unsafe_set_guid(cref<actor_guid> guid_) {
    _guid = guid_;
}

ptr<ActorComponent> Actor::getRootComponent() const noexcept {
    return _rootComponent;
}

cref<Transform> Actor::getWorldTransform() const noexcept {
    if (_rootComponent != nullptr) {
        return _rootComponent->getWorldTransform();
    }

    // TODO: Cleanup dangling reference within engine frontend
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

ptr<Actor> ember::CreateActor(cref<Session> session_) {
    const auto* session { static_cast<ptr<engine::core::Session>>(session_.unwrap().get()) };
    auto* registry { session->getState()->getRegistry() };

    auto* actor { registry->createActor() };
    return actor;
}

ptr<Actor> ember::CreateActor(cref<World> activeWorld_) {
    const auto* world { activeWorld_.unwrap().get() };

    const auto* engine { engine::Engine::getEngine() };
    const auto ctxs { engine->getWorldContexts() };// TODO: Check whether really want to copy the set

    const auto where = std::ranges::find_if(ctxs, [world](const auto& ctx_) {
        return ctx_->getCurrentWorld().get() == world;
    });

    if (where == _STD ranges::end(ctxs)) [[unlikely]]
    {
        return nullptr;
    }

    const auto* session { (*where)->getOwner() };
    auto* registry { session->getState()->getRegistry() };

    auto* actor { registry->createActor() };
    return actor;
}

Future<ptr<Actor>> ember::CreateActor(cref<Session> session_, async_t) {
    const auto* session { static_cast<ptr<engine::core::Session>>(session_.unwrap().get()) };

    concurrent::promise<ptr<Actor>> p {
        [session]() {
            auto* registry { session->getState()->getRegistry() };

            auto* actor { registry->createActor() };
            return actor;
        }
    };

    auto f { p.get() };
    p();

    return Future { _STD move(f) };
}

Future<ptr<Actor>> ember::CreateActor(cref<World> activeWorld_, async_t) {
    const auto* world { activeWorld_.unwrap().get() };

    const auto* engine { engine::Engine::getEngine() };
    const auto ctxs { engine->getWorldContexts() };// TODO: Check whether really want to copy the set

    const auto where = std::ranges::find_if(ctxs, [world](const auto& ctx_) {
        return ctx_->getCurrentWorld().get() == world;
    });

    const auto* session { where == _STD ranges::end(ctxs) ? nullptr : (*where)->getOwner() };
    concurrent::promise<ptr<Actor>> p {
        [session]() -> ptr<Actor> {
            if (session) [[likely]]
            {
                auto* registry { session->getState()->getRegistry() };

                auto* actor { registry->createActor() };
                return actor;
            }

            return nullptr;
        }
    };

    auto f { p.get() };
    p();

    return Future { _STD move(f) };
}

Future<ptr<Actor>> ember::CreateActor(cref<ActorClass> class_, cref<Session> session_) noexcept {
    throw NotImplementedException();
}

Future<ptr<Actor>> ember::CreateActor(cref<ActorClass> class_, cref<World> activeWorld_) noexcept {
    throw NotImplementedException();
}

Future<ptr<Actor>> ember::CreateActor(cref<ActorClass> class_, const ptr<SerializedActor> serialized_,
    cref<Session> session_) noexcept {
    throw NotImplementedException();
}

Future<ptr<Actor>> ember::CreateActor(cref<ActorClass> class_, const ptr<SerializedActor> serialized_,
    cref<World> activeWorld_) noexcept {
    throw NotImplementedException();
}

Future<ptr<Actor>> ember::CloneActorInto(const ptr<Actor> actor_, cref<Session> session_) noexcept {
    throw NotImplementedException();
}

Future<ptr<Actor>> ember::CloneActorInto(const ptr<Actor> actor_, cref<World> activeWorld_) noexcept {
    throw NotImplementedException();
}

Future<ptr<Actor>> ember::SpawnActor(cref<ActorClass> class_, cref<World> activeWorld_) noexcept {
    throw NotImplementedException();
}

Future<ptr<Actor>> ember::SpawnActor(cref<ActorClass> class_, const ptr<SerializedActor> serialized_,
    cref<World> activeWorld_) noexcept {
    throw NotImplementedException();
}

Future<bool> ember::Destroy(cref<Session> session_, mref<ptr<Actor>> actor_) noexcept {
    const auto* const session { static_cast<ptr<engine::core::Session>>(session_.unwrap().get()) };
    auto* const registry { session->getState()->getRegistry() };

    const auto guid { actor_->guid() };
    // Warning: Check modify-on-read condition
    // Warning: Multiple unsafe paths!!!
    for (const auto* const component : actor_->getComponents()) {
        assert(component->getTypeId().data);
        registry->releaseActorComponent(guid, component->getTypeId());
    }

    /**/
    registry->destroyActor(_STD move(actor_));

    /**/
    concurrent::promise<bool> p {
        []() {
            return true;
        }
    };

    auto f = p.get();
    p();

    return Future { _STD move(f) };
}

Future<bool> ember::Destroy(cref<World> activeWorld_, mref<ptr<Actor>> actor_) noexcept {
    const auto* world { activeWorld_.unwrap().get() };

    const auto* engine { engine::Engine::getEngine() };
    const auto ctxs { engine->getWorldContexts() };// TODO: Check whether really want to copy the set

    const auto where = std::ranges::find_if(ctxs, [world](const auto& ctx_) {
        return ctx_->getCurrentWorld().get() == world;
    });

    auto* coreSession { where == _STD ranges::end(ctxs) ? nullptr : (*where)->getOwner() };

    managed<void> dummy {};
    const Session session {
        managed<void> { _STD move(dummy), coreSession }
    };

    return Destroy(session, _STD move(actor_));
}
