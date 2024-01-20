#include "RenderSceneSystem.hpp"

#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX/Pool/SceneResourcePool.hpp>

using namespace hg::engine::render;
using namespace hg;

RenderSceneSystem::RenderSceneSystem() = default;

RenderSceneSystem::~RenderSceneSystem() noexcept = default;

void RenderSceneSystem::prepare() {

    assert(_sceneResourcePool == nullptr);

    const auto* const graphics = Engine::getEngine()->getGraphics();
    _sceneResourcePool = make_uptr<gfx::SceneResourcePool>(graphics->getCurrentDevice());

    _sceneResourcePool->setup();
}

void RenderSceneSystem::cleanup() {

    if (_sceneResourcePool != nullptr) {
        _sceneResourcePool->destroy();
        _sceneResourcePool.reset();
    }

}

nmpt<engine::gfx::SceneResourcePool> RenderSceneSystem::getSceneResourcePool() const noexcept {
    return _sceneResourcePool.get();
}

void RenderSceneSystem::broadcast(scene::SceneBroadcastFlags flags_) {}

void RenderSceneSystem::update(scene::SceneUpdateFlags flags_) {

    for (auto& storage : _storage.storageMap.values()) {
        storage.second->forEachMut(
            [this](ref<RenderSceneSystemModel> model_) {
                model_.update(this);
            }
        );
    }

}

void RenderSceneSystem::postprocess(scene::ScenePostProcessFlags flags_) {}

ref<RenderSceneRegistry> RenderSceneSystem::getRegistry() noexcept {
    return _storage;
}

void RenderSceneSystem::add(const ptr<const MetaClass> meta_, std::span<const ptr<SceneComponent>> batch_) {

    for (const auto& mapping : _storage.mapping) {

        if (mapping.first != meta_) {
            continue;
        }

        auto* const storage = _storage.storageMap.at(mapping.second).get();
        for (auto* const src : batch_) {
            auto model = storage->add(src);
            model->create(this);
        }
    }

}

void RenderSceneSystem::remove(const ptr<const MetaClass> meta_, std::span<const ptr<const SceneComponent>> batch_) {
    std::unreachable();
}

void RenderSceneSystem::clear() {
    std::unreachable();
}
