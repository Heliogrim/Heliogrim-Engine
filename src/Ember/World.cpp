#include "World.hpp"

#include <Engine.Common/Concurrent/Promise.hpp>
#include <Engine.Scene/SceneFactory.hpp>
#include <Engine.Core/Event/WorldAddedEvent.hpp>
#include <Engine.Core/Event/WorldChangeEvent.hpp>
#include <Engine.Core/Event/WorldRemoveEvent.hpp>

#include "Scene.hpp"

using namespace ember;

World::World(const non_owning_rptr<void> internal_) :
    _internal(internal_) {}

cref<non_owning_rptr<void>> World::unwrap() const noexcept {
    return _internal;
}

bool World::addLevel(const ptr<Level> level_) {
    throw NotImplementedException {};
}

bool World::removeLevel(const ptr<Level> level_) {
    throw NotImplementedException {};
}

bool World::addActor(const ptr<Actor> actor_) {
    /**
     *
     */
    const ptr<IComponentRegisterContext> ctx { static_cast<ptr<Scene>>(_internal)->registerContext() };
    actor_->registerComponents(ctx);

    /**
     *
     */
    return true;
}

bool World::removeActor(const ptr<Actor> actor_) {
    /**
     *
     */
    const ptr<IComponentRegisterContext> ctx { static_cast<ptr<Scene>>(_internal)->registerContext() };
    // actor_->unregisterComponents(ctx);

    /**
     *
     */
    return true;
}

Future<ptr<World>> ember::CreateWorld() noexcept {

    auto prom {
        ember::concurrent::promise<ptr<World>>([]() {

            const auto scene { ::ember::engine::scene::SceneFactory::createDefaultScene() };
            engine::Session::get()->core().addScene(scene);
            engine::Session::get()->emitter().emit<::ember::engine::core::WorldAddedEvent>(scene);

            return scene->getWorld();
        })
    };

    auto fut { prom.get() };
    execute(prom);

    return fut;
}

const ptr<World> ember::GetWorld() noexcept {
    const auto& core { engine::Session::get()->core() };

    if (core.getCurrentScene()) {
        return core.getCurrentScene()->getWorld();
    }

    return nullptr;
}

Future<ptr<World>> ember::GetWorld(cref<asset_guid> guid_) noexcept {
    throw NotImplementedException {};
}

Future<bool> ember::Destroy(mref<ptr<World>> world_) {

    auto prom {
        ember::concurrent::promise<bool>([world = _STD move(world_)]() {

            ptr<Scene> scene { static_cast<ptr<Scene>>(world->unwrap()) };

            auto& core { engine::Session::get()->core() };
            const auto& emitter { engine::Session::get()->emitter() };

            if (core.getCurrentScene() == scene) {
                emitter.emit<::ember::engine::core::WorldChangeEvent>(core.getCurrentScene(), nullptr);
                core.setCurrentScene(nullptr);
            }

            emitter.emit<::ember::engine::core::WorldRemoveEvent>(scene);
            core.removeScene(_STD move(scene));

            return true;
        })
    };

    auto fut { prom.get() };
    execute(prom);

    return fut;

}

void ember::SetWorld(const ptr<World> world_) {

    if (not world_ || world_ == GetWorld()) {
        return;
    }

    auto& core { engine::Session::get()->core() };
    const auto& emitter { engine::Session::get()->emitter() };

    const auto nextScene { core.resolveScene(world_) };
    if (not nextScene) {
        return;
    }

    emitter.emit<::ember::engine::core::WorldChangeEvent>(core.getCurrentScene(), nextScene);
    engine::Session::get()->core().setCurrentScene(nextScene.get());
}

void ember::TransitionToWorld(const ptr<World> world_) {

    // TODO:
    SetWorld(world_);
}
