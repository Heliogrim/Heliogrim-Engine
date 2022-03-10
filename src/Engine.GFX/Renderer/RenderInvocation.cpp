#include "RenderInvocation.hpp"

#include "Renderer.hpp"
#include "RenderInvocationData.hpp"

using namespace ember::engine::gfx;
using namespace ember;

RenderInvocation::RenderInvocation(const non_owning_rptr<Renderer> renderer_, RenderInvocationData data_,
    const sptr<RenderInvocationState> state_) :
    _scene(data_.scene),
    _camera(data_.camera),
    _target(data_.target),
    _renderer(renderer_),
    _state(state_) {}

RenderInvocation::~RenderInvocation() {
    tidy();
}

void RenderInvocation::tidy() {

    if (_sync.load()) {
        auto* fence { reinterpret_cast<ptr<vk::Fence>>(_sync.load()) };

        _renderer->device()->vkDevice().destroyFence(*fence);
        delete fence;

        _sync.store(NULL);
    }
}

const ptr<engine::scene::IRenderScene> RenderInvocation::scene() const noexcept {
    return _scene;
}

bool RenderInvocation::use(const ptr<scene::IRenderScene> scene_) noexcept {

    if (!await()) {
        return false;
    }

    _scene = scene_;
    return true;
}

const ptr<Camera> RenderInvocation::camera() const noexcept {
    return _camera;
}

bool RenderInvocation::use(const ptr<Camera> camera_) noexcept {

    if (!await()) {
        return false;
    }

    _camera = camera_;
    return true;
}

const ptr<Texture> RenderInvocation::target() const noexcept {
    return _target;
}

const non_owning_rptr<Renderer> RenderInvocation::renderer() const noexcept {
    return _renderer;
}

cref<sptr<RenderInvocationState>> RenderInvocation::state() const noexcept {
    return _state;
}

bool RenderInvocation::storeSync(mref<vk::Fence> fence_) {

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

vk::Fence RenderInvocation::unsafeSync() const noexcept {

    const auto value { _sync.load() };
    if (!value) {
        return vk::Fence {};
    }

    return *reinterpret_cast<ptr<vk::Fence>>(value);
}

bool RenderInvocation::await() const noexcept {

    auto value { _sync.load() };
    if (!value) {
        return true;
    }

    auto* fence { reinterpret_cast<ptr<vk::Fence>>(value) };
    return _renderer->device()->vkDevice().getFenceStatus(*fence) != vk::Result::eNotReady;
}
