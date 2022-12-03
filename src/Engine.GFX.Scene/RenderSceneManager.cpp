#include "RenderSceneManager.hpp"

#include <Engine.GFX/RenderTarget.hpp>

using namespace ember::engine::gfx::scene;
using namespace ember::engine::gfx;
using namespace ember;

uptr<RenderSceneManager> RenderSceneManager::_instance = nullptr;

non_owning_rptr<RenderSceneManager::this_type> RenderSceneManager::get() noexcept {
    return _instance.get();
}

non_owning_rptr<RenderSceneManager::this_type> RenderSceneManager::make() {

    if (not _instance) {
        _instance = uptr<RenderSceneManager>(new RenderSceneManager());
    }

    return _instance.get();
}

void RenderSceneManager::destroy() {
    _instance.reset();
}

RenderSceneManager::RenderSceneManager() = default;

RenderSceneManager::~RenderSceneManager() = default;

void RenderSceneManager::registerScene(const ptr<RenderScene> renderScene_) {
    _CTRL_GATE(_ctrl);
    _renderScenes.insert(renderScene_);
    __gate_lock.unlock();

    injectSceneHooks(renderScene_);
}

bool RenderSceneManager::unregisterScene(const ptr<RenderScene> renderScene_) {
    _CTRL_GATE(_ctrl);
    return _renderScenes.erase(renderScene_) != 0;
}

void RenderSceneManager::addPrimaryTarget(cref<sptr<RenderTarget>> renderTarget_) {
    _primaryTargets.insert(renderTarget_);
}

void RenderSceneManager::dropPrimaryTarget(const sptr<RenderTarget> renderTarget_) {
    _primaryTargets.erase(renderTarget_);
}

void RenderSceneManager::addSecondaryTarget(cref<sptr<RenderTarget>> renderTarget_) {
    _secondaryTargets.insert(renderTarget_);
}

void RenderSceneManager::dropSecondaryTarget(const sptr<RenderTarget> renderTarget_) {
    _secondaryTargets.erase(renderTarget_);
}

void RenderSceneManager::selectInvokeTargets(ref<CompactSet<sptr<RenderTarget>>> targets_) const noexcept {
    targets_.reserve(_primaryTargets.size());
    for (const auto& entry : _primaryTargets) {
        if (entry->active() && entry->ready()) {
            targets_.insert(entry);
        }
    }
}
