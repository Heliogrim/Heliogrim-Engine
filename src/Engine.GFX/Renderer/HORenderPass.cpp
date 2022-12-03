#include "HORenderPass.hpp"

#include <Engine.Scene/IRenderScene.hpp>

#include "HORenderPassCreateData.hpp"
#include "Renderer.hpp"
#include "RenderPassState.hpp"
#include "../Command/CommandBatch.hpp"
#include "../Texture/Texture.hpp"

using namespace ember::engine::gfx::render;
using namespace ember::engine::gfx;
using namespace ember;

HORenderPass::HORenderPass(
    const non_owning_rptr<Renderer> renderer_,
    HORenderPassCreateData data_,
    cref<sptr<RenderPassState>> state_
) :
    _reset(),
    _scene(data_.scene),
    _sceneView(data_.sceneView),
    _target(data_.target),
    _renderer(renderer_),
    _state(state_) {

    /**
     * Default is reset state
     */
    _reset.test_and_set(_STD memory_order::relaxed);
}

HORenderPass::HORenderPass(mref<this_type> other_) noexcept :
    _reset(),
    _scene(other_._scene),
    _sceneView(other_._sceneView),
    _target(_STD exchange(other_._target, nullptr)),
    _renderer(_STD exchange(other_._renderer, nullptr)),
    _state(_STD exchange(other_._state, nullptr)),
    _lastSignals(_STD move(other_._lastSignals)),
    _sync(other_._sync.exchange(reinterpret_cast<ptrdiff_t>(nullptr))) {

    /**
     * Copy reset state from other instance
     */
    if (other_._reset.test(_STD memory_order::consume)) {
        _reset.test_and_set(_STD memory_order::relaxed);
        other_._reset.clear(_STD memory_order::relaxed);
    }
}

HORenderPass::~HORenderPass() {
    tidy();
}

void HORenderPass::tidy() {
    clearSync();
}

bool HORenderPass::isReset() const noexcept {
    return _reset.test(_STD memory_order::consume);
}

void HORenderPass::reset() {

    /**
     * Check pending state
     */
    if (!await()) {
        return;
    }

    /**
     * Check whether state is already reset
     */
    if (_reset.test_and_set()) {
        return;
    }

    /**
     * Reset
     */
    auto* state { _state.get() };
    // TODO: Check whether state should get a reset method, which forwards invocation to state's members
    state->cacheCtrl.reset();

    clearSync();
}

void HORenderPass::markAsTouched() {
    _reset.clear(_STD memory_order::release);
}

const ptr<engine::scene::IRenderScene> HORenderPass::scene() const noexcept {
    return _scene;
}

bool HORenderPass::use(const ptr<RenderScene> scene_) noexcept {

    if (!await()) {
        return false;
    }

    _scene = scene_;
    return true;
}

const ptr<scene::SceneView> HORenderPass::sceneView() const noexcept {
    return _sceneView;
}

bool HORenderPass::use(const ptr<scene::SceneView> sceneView_) noexcept {

    if (not await()) {
        return false;
    }

    _sceneView = sceneView_;
    return true;
}

cref<sptr<Texture>> HORenderPass::target() const noexcept {
    return _target;
}

const non_owning_rptr<Renderer> HORenderPass::renderer() const noexcept {
    return _renderer;
}

cref<sptr<RenderPassState>> HORenderPass::state() const noexcept {
    return _state;
}

sptr<RenderPassState> HORenderPass::state() noexcept {
    return _state;
}

ref<decltype(HORenderPass::_batches)> HORenderPass::batches() noexcept {
    return _batches;
}

ref<decltype(HORenderPass::_lastSignals)> HORenderPass::lastSignals() noexcept {
    return _lastSignals;
}

void HORenderPass::clearSync() {

    if (!_sync.load()) {
        return;
    }

    auto* fence { reinterpret_cast<ptr<vk::Fence>>(_sync.load()) };

    _renderer->device()->vkDevice().destroyFence(*fence);
    delete fence;

    _sync.store(NULL);
}

bool HORenderPass::storeSync(mref<vk::Fence> fence_) {

    /**
     * Precheck
     */
    if (_sync.load()) {
        return false;
    }

    /**
     * Prepare
     */
    auto* fence { new vk::Fence(_STD move(fence_)) };
    auto value { reinterpret_cast<ptrdiff_t>(fence) };

    /**
     * Exchange
     */
    ptrdiff_t expect {};
    if (_sync.compare_exchange_strong(expect, value)) {
        return true;
    }

    /**
     * Cleanup on fail
     */
    _renderer->device()->vkDevice().destroyFence(*fence);
    delete fence;

    return false;
}

vk::Fence HORenderPass::unsafeSync() const noexcept {

    const auto value { _sync.load() };
    if (!value) {
        return vk::Fence {};
    }

    return *reinterpret_cast<ptr<vk::Fence>>(value);
}

bool HORenderPass::await() const noexcept {

    auto value { _sync.load() };
    if (!value) {
        return true;
    }

    auto* fence { reinterpret_cast<ptr<vk::Fence>>(value) };
    return _renderer->device()->vkDevice().getFenceStatus(*fence) != vk::Result::eNotReady;
}
