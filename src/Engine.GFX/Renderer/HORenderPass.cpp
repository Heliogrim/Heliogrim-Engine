#include "HORenderPass.hpp"

#include <Engine.Scene/IRenderScene.hpp>

#include "HORenderPassCreateData.hpp"
#include "Renderer.hpp"
#include "RenderPassState.hpp"
#include "../Camera/Camera.hpp"
#include "../Command/CommandBatch.hpp"
#include "../Texture/Texture.hpp"

using namespace ember::engine::gfx::render;
using namespace ember::engine::gfx;
using namespace ember;

HORenderPass::HORenderPass(const non_owning_rptr<Renderer> renderer_, HORenderPassCreateData data_,
    cref<sptr<RenderPassState>> state_) :
    _scene(data_.scene),
    _camera(data_.camera),
    _target(data_.target),
    _renderer(renderer_),
    _state(state_) {}

HORenderPass::HORenderPass(mref<this_type> other_) noexcept :
    _scene(other_._scene),
    _camera(other_._camera),
    _target(_STD exchange(other_._target, nullptr)),
    _renderer(_STD exchange(other_._renderer, nullptr)),
    _state(_STD exchange(other_._state, nullptr)),
    _lastSignals(_STD move(other_._lastSignals)),
    _sync(other_._sync.exchange(reinterpret_cast<ptrdiff_t>(nullptr))) {}

HORenderPass::~HORenderPass() {
    tidy();
}

void HORenderPass::tidy() {

    if (_sync.load()) {
        auto* fence { reinterpret_cast<ptr<vk::Fence>>(_sync.load()) };

        _renderer->device()->vkDevice().destroyFence(*fence);
        delete fence;

        _sync.store(NULL);
    }
}

const ptr<engine::scene::IRenderScene> HORenderPass::scene() const noexcept {
    return _scene;
}

bool HORenderPass::use(const ptr<scene::IRenderScene> scene_) noexcept {

    if (!await()) {
        return false;
    }

    _scene = scene_;
    return true;
}

const ptr<Camera> HORenderPass::camera() const noexcept {
    return _camera;
}

bool HORenderPass::use(const ptr<Camera> camera_) noexcept {

    if (!await()) {
        return false;
    }

    _camera = camera_;
    return true;
}

const ptr<Texture> HORenderPass::target() const noexcept {
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
