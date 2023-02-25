#include "World.hpp"

#include <Engine.Common/Concurrent/Promise.hpp>
#include <Engine.Scene/SceneFactory.hpp>
#include <Engine.Core/Event/WorldAddedEvent.hpp>
#include <Engine.Core/Event/WorldChangeEvent.hpp>
#include <Engine.Core/Event/WorldRemoveEvent.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Session.hpp>
#include <Engine.Core/WorldContext.hpp>
#include <Engine.Core/World.hpp>
#include <Engine.Scene/Scene.hpp>

#include "Scene.hpp"
#include "Session.hpp"

using namespace ember;

World::World() = default;

World::World(const managed<void> internal_) :
    _internal(internal_) {}

bool World::valid() const noexcept {
    return _internal.get() && _internal.use_count();
}

cref<managed<void>> World::unwrap() const noexcept {
    return _internal;
}

bool World::addLevel(const ptr<Level> level_) {
    throw NotImplementedException {};
}

bool World::removeLevel(const ptr<Level> level_) {
    throw NotImplementedException {};
}

bool World::addActor(const ptr<Actor> actor_) {
    const auto* const world { static_cast<ptr<engine::core::World>>(_internal.get()) };
    auto* const scene { world->getScene() };
    auto* const ctx { scene->registerContext() };
    actor_->registerComponents(ctx);

    /**
     *
     */
    return true;
}

bool World::removeActor(const ptr<Actor> actor_) {
    const auto* const world { static_cast<ptr<engine::core::World>>(_internal.get()) };
    auto* const scene { world->getScene() };
    auto* const ctx { scene->registerContext() };
    // actor_->unregisterComponents(ctx);

    /**
     *
     */
    return true;
}

Future<World> ember::CreateWorld() noexcept {
    auto prom {
        ember::concurrent::promise<World>(
            []() {
                auto scene { ::ember::engine::scene::SceneFactory::createDefaultScene() };
                const auto world { make_sptr<engine::core::World>(_STD move(scene)) };

                engine::Engine::getEngine()->addWorld(world);

                return World { world };
            }
        )
    };

    auto fut { prom.get() };
    execute(prom);

    return fut;
}

World ember::GetWorld(cref<Session> session_) noexcept {
    const auto* session { static_cast<ptr<::ember::engine::core::Session>>(session_.unwrap().get()) };
    const auto* context { session->getWorldContext() };
    const auto world { context->getCurrentWorld() };

    return World { world };
}

World ember::GetWorld() noexcept {
    return GetWorld(GetSession());
}

Future<World> ember::GetWorld(cref<asset_guid> guid_) noexcept {
    throw NotImplementedException {};
}

Future<bool> ember::Destroy(mref<World> world_) {
    auto prom {
        ember::concurrent::promise<bool>(
            [world_ = _STD move(world_)]() {
                const auto world { _STD static_pointer_cast<engine::core::World, void>(world_.unwrap()) };

                // TODO: Remove world from every context / session where it might be used currently
                // TODO: Check whether we want to auto handle session's world transition when engine propagated world erase happens

                engine::Engine::getEngine()->removeWorld(world);

                return true;
            }
        )
    };

    auto fut { prom.get() };
    execute(prom);

    return fut;
}

void ember::SetWorld(cref<Session> session_, cref<World> world_) {
    const auto* const session { static_cast<ptr<engine::core::Session>>(session_.unwrap().get()) };
    auto* const context { session->getWorldContext() };
    const sptr<engine::core::World> world { _STD static_pointer_cast<engine::core::World, void>(world_.unwrap()) };

    if (context->getCurrentWorld() == world) {
        return;
    }

    context->setCurrentWorld(world);
}

void ember::TransitionToWorld(cref<Session> session_, cref<World> world_) {
    // TODO:
    SetWorld(session_, world_);
}
