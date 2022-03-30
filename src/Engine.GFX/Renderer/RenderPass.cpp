#include "RenderPass.hpp"

#include "Renderer.hpp"
#include "RenderPassCreateData.hpp"

using namespace ember::engine::gfx;
using namespace ember;

RenderPass::RenderPass(const non_owning_rptr<Renderer> renderer_, RenderPassCreateData data_,
    const sptr<RenderPassState> state_) :
    _scene(data_.scene),
    _camera(data_.camera),
    _target(data_.target),
    _renderer(renderer_),
    _state(state_) {}

RenderPass::~RenderPass() {
    tidy();
}

void RenderPass::tidy() {

    if (_sync.load()) {
        auto* fence { reinterpret_cast<ptr<vk::Fence>>(_sync.load()) };

        _renderer->device()->vkDevice().destroyFence(*fence);
        delete fence;

        _sync.store(NULL);
    }
}

const ptr<engine::scene::IRenderScene> RenderPass::scene() const noexcept {
    return _scene;
}

bool RenderPass::use(const ptr<scene::IRenderScene> scene_) noexcept {

    if (!await()) {
        return false;
    }

    _scene = scene_;
    return true;
}

const ptr<Camera> RenderPass::camera() const noexcept {
    return _camera;
}

bool RenderPass::use(const ptr<Camera> camera_) noexcept {

    if (!await()) {
        return false;
    }

    _camera = camera_;
    return true;
}

const ptr<Texture> RenderPass::target() const noexcept {
    return _target;
}

const non_owning_rptr<Renderer> RenderPass::renderer() const noexcept {
    return _renderer;
}

cref<sptr<RenderPassState>> RenderPass::state() const noexcept {
    return _state;
}

ref<decltype(RenderPass::_lastSignals)> RenderPass::lastSignals() noexcept {
    return _lastSignals;
}

bool RenderPass::storeSync(mref<vk::Fence> fence_) {

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

vk::Fence RenderPass::unsafeSync() const noexcept {

    const auto value { _sync.load() };
    if (!value) {
        return vk::Fence {};
    }

    return *reinterpret_cast<ptr<vk::Fence>>(value);
}

bool RenderPass::await() const noexcept {

    auto value { _sync.load() };
    if (!value) {
        return true;
    }

    auto* fence { reinterpret_cast<ptr<vk::Fence>>(value) };
    return _renderer->device()->vkDevice().getFenceStatus(*fence) != vk::Result::eNotReady;
}
