#include "RenderPass.hpp"

#include <Engine.GFX.RenderGraph/RuntimeGraph.hpp>
#include <Engine.GFX.RenderGraph/Pass/ExecutionPass.hpp>
#include <Engine.GFX.RenderGraph/Relation/TextureDescription.hpp>
#include <Engine.GFX.RenderPipeline/RenderPipeline.hpp>
#include <Engine.GFX.RenderPipeline/State/State.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Reflect/Cast.hpp>

using namespace hg::engine::gfx::render;
using namespace hg;

RenderPass::RenderPass(
    mref<nmpt<const Renderer>> renderer_,
    mref<uptr<graph::RuntimeGraph>> runtimeGraph_,
    mref<smr<cache::LocalCacheCtrl>> localGeneralCache_,
    mref<smr<memory::LocalPooledAllocator>> localDeviceAllocator_
) noexcept :
    _renderer(_STD move(renderer_)),
    _state(_STD move(localGeneralCache_), _STD move(localDeviceAllocator_)),
    _graph(_STD move(runtimeGraph_)) {}

RenderPass::~RenderPass() noexcept = default;

bool RenderPass::alloc() {
    //return _pipeline->alloc(_state.get());
    return true;
}

bool RenderPass::realloc() {
    //if (_pipeline->isReallocSupported()) {
    //    return _pipeline->realloc(_state.get());
    //}

    //const auto success = free();
    //return success ? alloc() : false;
    return true;
}

bool RenderPass::free() {

    //_state._gfxCache->reset(true);
    //return _pipeline->free(_state.get());
    return true;
}

graph::ExecutionPass RenderPass::getExecutionPass() noexcept {
    return graph::ExecutionPass {};
}

RenderPassResult RenderPass::operator()() {

    /* Execute Runtime Graph */
    auto pass = getExecutionPass();
    auto execution = pass.visitor();

    _graph->update(execution);

    return RenderPassResult::eSuccess;
}

nmpt<engine::gfx::scene::SceneView> RenderPass::changeSceneView(mref<nmpt<scene::SceneView>> nextSceneView_) {
    return _STD exchange(_state._sceneView, _STD move(nextSceneView_));
}

void RenderPass::unsafeBindTarget(mref<smr<const acc::Symbol>> target_, mref<nmpt<void>> resource_) {
    _state._boundTargets.insert_or_assign(target_, _STD move(resource_));
}

bool RenderPass::bindTarget(mref<smr<const acc::Symbol>> target_, mref<nmpt<Texture>> texture_) {

    const auto* const textureDescription = Cast<graph::TextureDescription>(target_->description.get());
    if (not textureDescription) {
        return false;
    }

    if (not textureDescription->isValidTexture(clone(texture_))) {
        return false;
    }

    _state._boundTargets.insert_or_assign(target_, _STD move(texture_));
    return true;
}

bool RenderPass::bindTarget(mref<smr<const acc::Symbol>> target_, mref<nmpt<TextureView>> textureView_) {

    const auto* const textureDescription = Cast<graph::TextureDescription>(target_->description.get());
    if (not textureDescription) {
        return false;
    }

    if (not textureDescription->isValidTexture(clone(textureView_))) {
        return false;
    }

    _state._boundTargets.insert_or_assign(target_, _STD move(textureView_));
    return true;
}

bool RenderPass::bindTarget(mref<smr<const acc::Symbol>> target_, mref<nmpt<VirtualTexture>> texture_) {

    const auto* const textureDescription = Cast<graph::TextureDescription>(target_->description.get());
    if (not textureDescription) {
        return false;
    }

    if (not textureDescription->isValidTexture(clone(texture_))) {
        return false;
    }

    _state._boundTargets.insert_or_assign(target_, _STD move(texture_));
    return true;
}

bool RenderPass::bindTarget(mref<smr<const acc::Symbol>> target_, mref<nmpt<VirtualTextureView>> textureView_) {

    const auto* const textureDescription = Cast<graph::TextureDescription>(target_->description.get());
    if (not textureDescription) {
        return false;
    }

    if (not textureDescription->isValidTexture(clone(textureView_))) {
        return false;
    }

    _state._boundTargets.insert_or_assign(target_, _STD move(textureView_));
    return true;
}

nmpt<void> RenderPass::unbindTarget(mref<smr<const acc::Symbol>> target_) noexcept {

    const auto iter = _state._boundTargets.find(target_);
    if (iter == _state._boundTargets.end()) {
        return nullptr;
    }

    auto stored = _STD move(iter->second);
    _state._boundTargets.erase(iter);

    return stored;
}

/**/

#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX/Device/Device.hpp>

void RenderPass::clearSync() {

    if (!_sync.load()) {
        return;
    }

    auto* fence { reinterpret_cast<ptr<vk::Fence>>(_sync.load()) };

    Engine::getEngine()->getGraphics()->getCurrentDevice()->vkDevice().destroyFence(*fence);
    delete fence;

    _sync.store(NULL);
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
    Engine::getEngine()->getGraphics()->getCurrentDevice()->vkDevice().destroyFence(*fence);
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
    return Engine::getEngine()->getGraphics()->getCurrentDevice()->vkDevice().getFenceStatus(*fence) !=
        vk::Result::eNotReady;
}

bool RenderPass::isReset() const noexcept {
    return _reset.test(_STD memory_order::consume);
}

void RenderPass::reset() {

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
    // TODO: Check whether state should get a reset method, which forwards invocation to state's members
    _state._gfxCache->reset();

    clearSync();
}

void RenderPass::markAsTouched() {
    _reset.clear(_STD memory_order::release);
}

ref<decltype(RenderPass::_targetWaitSignals)> RenderPass::getTargetWaitSignals() noexcept {
    return _targetWaitSignals;
}

ref<decltype(RenderPass::_targetWaitSignalStages)> RenderPass::getTargetWaitSignalStages() noexcept {
    return _targetWaitSignalStages;
}

ref<decltype(RenderPass::_targetReadySignals)> RenderPass::getTargetReadySignals() noexcept {
    return _targetReadySignals;
}
