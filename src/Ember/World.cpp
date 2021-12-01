#include "World.hpp"

using namespace ember;

World::World(const non_owning_rptr<Scene> scene_) :
    _scene(scene_) {}

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
    const ptr<IComponentRegisterContext> ctx { _scene->registerContext() };
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
    const ptr<IComponentRegisterContext> ctx { _scene->registerContext() };
    //actor_->unregisterComponents(ctx);

    /**
     *
     */
    return true;
}

Future<ptr<World>> ember::CreateWorld() noexcept {
    throw NotImplementedException {};
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
    throw NotImplementedException {};
}
