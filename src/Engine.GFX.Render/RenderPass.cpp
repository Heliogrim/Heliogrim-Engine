#include "RenderPass.hpp"

#include <ranges>
#include <Engine.GFX.Render.Predefined/Symbols/SceneView.hpp>
#include <Engine.GFX.RenderGraph/RuntimeGraph.hpp>
#include <Engine.GFX.RenderGraph/Pass/ExecutionPass.hpp>
#include <Engine.GFX.RenderGraph/Relation/SceneViewDescription.hpp>
#include <Engine.GFX.RenderGraph/Relation/TextureDescription.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.GFX/Texture/Texture.hpp>
#include <Engine.GFX/Texture/TextureView.hpp>
#include <Engine.GFX/Texture/SparseTexture.hpp>
#include <Engine.GFX/Texture/SparseTextureView.hpp>
#include <Engine.Logging/Logger.hpp>

using namespace hg::engine::render;
using namespace hg;

RenderPass::RenderPass(
    mref<nmpt<const Renderer>> renderer_,
    mref<uptr<graph::RuntimeGraph>> runtimeGraph_,
    mref<smr<gfx::cache::LocalCacheCtrl>> localGeneralCache_,
    mref<smr<gfx::memory::LocalPooledAllocator>> localDeviceAllocator_
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
    return graph::ExecutionPass {
        _state.rootSymbolContext()
    };
}

RenderPassResult RenderPass::operator()() {

    /* Execute Runtime Graph */
    auto pass = getExecutionPass();
    auto execution = pass.visitor();

    _graph->update(execution);

    return RenderPassResult::eSuccess;
}

smr<const engine::gfx::scene::SceneView> RenderPass::changeSceneView(
    mref<smr<const gfx::scene::SceneView>> nextSceneView_
) {

    const auto symbol = makeSceneViewSymbol();

    auto stored = _state.rootSymbolContext().getExportSymbol(clone(symbol));
    if (stored == nullptr) {

        stored = _state.rootSymbolContext().exportSymbol(clone(symbol));
        stored->create<smr<const gfx::scene::SceneView>, graph::SceneViewDescription>(clone(nextSceneView_));

    } else {
        stored->store<smr<const gfx::scene::SceneView>, graph::SceneViewDescription>(clone(nextSceneView_));
    }

    return _STD exchange(_state._sceneView, _STD move(nextSceneView_));
}

smr<const engine::gfx::scene::SceneView> RenderPass::unbindSceneView() {

    const auto symbol = makeSceneViewSymbol();

    auto stored = _state.rootSymbolContext().getExportSymbol(clone(symbol));
    if (stored == nullptr) {
        return nullptr;
    }

    if (stored->empty()) {
        _state.rootSymbolContext().eraseExportSymbol(clone(symbol));
        return nullptr;
    }

    auto report = stored->load<smr<const gfx::scene::SceneView>>();
    stored->destroy<smr<const gfx::scene::SceneView>>();

    return report;
}

void RenderPass::unsafeBindTarget(mref<smr<const graph::Symbol>> target_, mref<smr<void>> resource_) {

    if (_state._boundTargets.contains(target_)) {
        IM_CORE_ERRORF("Tried to bind target `{}` while implicit rebinding is not allowed.", target_->name);
        return;
    }

    _state._boundTargets.insert_or_assign(clone(target_), clone(resource_));

    auto storage = _state.rootSymbolContext().exportSymbol(_STD move(target_));
    storage->create(_STD move(resource_));
}

bool RenderPass::bindTarget(mref<smr<const graph::Symbol>> target_, mref<smr<gfx::Texture>> texture_) {

    const auto* const textureDescription = Cast<graph::TextureDescription>(target_->description.get());
    if (not textureDescription) {
        return false;
    }

    if (not textureDescription->isValidObject(texture_.get())) {
        return false;
    }

    if (_state._boundTargets.contains(target_)) {
        IM_CORE_ERRORF("Tried to bind target `{}` while implicit rebinding is not allowed.", target_->name);
        return false;
    }

    _state._boundTargets.insert_or_assign(clone(target_), clone(texture_));

    auto storage = _state.rootSymbolContext().exportSymbol(_STD move(target_));
    storage->create(texture_.into<gfx::TextureLikeObject>());

    return true;
}

bool RenderPass::bindTarget(mref<smr<const graph::Symbol>> target_, mref<smr<gfx::TextureView>> textureView_) {

    const auto* const textureDescription = Cast<graph::TextureDescription>(target_->description.get());
    if (not textureDescription) {
        return false;
    }

    if (not textureDescription->isValidObject(textureView_.get())) {
        return false;
    }

    if (_state._boundTargets.contains(target_)) {
        IM_CORE_ERRORF("Tried to bind target `{}` while implicit rebinding is not allowed.", target_->name);
        return false;
    }

    _state._boundTargets.insert_or_assign(clone(target_), clone(textureView_));

    auto storage = _state.rootSymbolContext().exportSymbol(_STD move(target_));
    storage->create(textureView_.into<gfx::TextureLikeObject>());

    return true;
}

bool RenderPass::bindTarget(mref<smr<const graph::Symbol>> target_, mref<smr<gfx::SparseTexture>> texture_) {

    const auto* const textureDescription = Cast<graph::TextureDescription>(target_->description.get());
    if (not textureDescription) {
        return false;
    }

    if (not textureDescription->isValidObject(texture_.get())) {
        return false;
    }

    if (_state._boundTargets.contains(target_)) {
        IM_CORE_ERRORF("Tried to bind target `{}` while implicit rebinding is not allowed.", target_->name);
        return false;
    }

    _state._boundTargets.insert_or_assign(clone(target_), clone(texture_));

    auto storage = _state.rootSymbolContext().exportSymbol(_STD move(target_));
    storage->create(texture_.into<gfx::TextureLikeObject>());

    return true;
}

bool RenderPass::bindTarget(mref<smr<const graph::Symbol>> target_, mref<smr<gfx::SparseTextureView>> textureView_) {

    const auto* const textureDescription = Cast<graph::TextureDescription>(target_->description.get());
    if (not textureDescription) {
        return false;
    }

    if (not textureDescription->isValidObject(textureView_.get())) {
        return false;
    }

    if (_state._boundTargets.contains(target_)) {
        IM_CORE_ERRORF("Tried to bind target `{}` while implicit rebinding is not allowed.", target_->name);
        return false;
    }

    _state._boundTargets.insert_or_assign(clone(target_), clone(textureView_));

    auto storage = _state.rootSymbolContext().exportSymbol(_STD move(target_));
    storage->create(textureView_.into<gfx::TextureLikeObject>());

    return true;
}

smr<void> RenderPass::unbindTarget(mref<smr<const graph::Symbol>> target_) noexcept {

    const auto iter = _state._boundTargets.find(target_);
    if (iter == _state._boundTargets.end()) {
        return nullptr;
    }

    auto stored = _STD move(iter->second);
    _state._boundTargets.erase(iter);

    auto storage = _state.rootSymbolContext().getExportSymbol(_STD move(target_));

    // TODO: ensure( storage->valid<>() );
    auto result = storage->load<smr<void>>() == stored;
    assert(result);

    storage->destroy<smr<void>>();
    result = _state.rootSymbolContext().eraseExportSymbol(_STD move(storage));
    assert(result);

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

bool RenderPass::addTargetWaitSignal(
    mref<smr<const graph::Symbol>> targetSymbol_,
    cref<vk::Semaphore> signal_
) noexcept {

    if (not _state._boundTargets.contains(targetSymbol_)) {
        return false;
    }

    const auto resource = _state.rootSymbolContext().getExportSymbol(clone(targetSymbol_));

    const auto valid = resource->valid<smr<const gfx::TextureLikeObject>,
        graph::TextureDescription,
        decltype([](auto&& obj_) {
            return obj_.get();
        })>();
    assert(valid);
    resource->barriers.push_back(signal_.operator VkSemaphore());

    return true;
}

void RenderPass::clearTargetWaitSignals(mref<smr<const graph::Symbol>> targetSymbol_) noexcept {

    if (not _state._boundTargets.contains(targetSymbol_)) {
        return;
    }

    const auto resource = _state.rootSymbolContext().getExportSymbol(clone(targetSymbol_));

    const auto valid = resource->valid<smr<const gfx::TextureLikeObject>,
        graph::TextureDescription,
        decltype([](auto&& obj_) {
            return obj_.get();
        })>();
    assert(valid);
    resource->barriers.clear();
}

void RenderPass::enumerateTargetWaitSignals(
    ref<Vector<vk::Semaphore>> signals_,
    mref<smr<const graph::Symbol>> targetSymbol_
) noexcept {

    // Warning: This is just temporary
    enumerateTargetReadySignals(signals_, _STD move(targetSymbol_));
}

void RenderPass::enumerateTargetReadySignals(
    ref<Vector<vk::Semaphore>> signals_,
    mref<smr<const graph::Symbol>> targetSymbol_
) noexcept {

    if (not targetSymbol_.empty()) {

        if (not _state._boundTargets.contains(targetSymbol_)) {
            return;
        }

        const auto resource = _state.rootSymbolContext().getExportSymbol(_STD move(targetSymbol_));
        for (auto* const barrier : resource->barriers) {
            signals_.emplace_back(static_cast<VkSemaphore>(barrier));
        }

        return;
    }

    /**/

    for (const auto& symbol : _state._boundTargets | std::views::keys) {

        auto resource = _state.rootSymbolContext().getExportSymbol(clone(symbol));
        for (auto* const barrier : resource->barriers) {
            signals_.emplace_back(static_cast<VkSemaphore>(barrier));
        }
    }

}
