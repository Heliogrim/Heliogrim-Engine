#include "RenderSceneManager.hpp"

#include <Engine.Common/Move.hpp>
#include <Engine.GFX/RenderTarget.hpp>
#include <Engine.GFX.Scene/View/SceneView.hpp>
#include <Engine.GFX/Swapchain/Swapchain.hpp>

using namespace hg::engine::gfx::scene;
using namespace hg::engine::gfx;
using namespace hg;

[[clang::always_destroy]] uptr<RenderSceneManager> RenderSceneManager::_instance = nullptr;

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

void RenderSceneManager::registerScene(nmpt<engine::scene::SceneBase> renderableScene_) {
	_CTRL_GATE(_ctrl);
	_renderScenes.insert(renderableScene_.get());
	__gate_lock.unlock();

	injectSceneHooks(renderableScene_);
}

bool RenderSceneManager::unregisterScene(nmpt<engine::scene::SceneBase> renderableScene_) {
	_CTRL_GATE(_ctrl);
	return _renderScenes.erase(renderableScene_.get()) != 0;
}

void RenderSceneManager::addPrimaryTarget(mref<smr<RenderTarget>> renderTarget_) {
	_primaryTargets.insert(clone(renderTarget_));
	_mappedRenderTargets.emplace(renderTarget_->getSwapChain(), std::move(renderTarget_));
}

void RenderSceneManager::dropPrimaryTarget(cref<smr<RenderTarget>> renderTarget_) {
	_mappedRenderTargets.erase(renderTarget_->getSwapChain());
	_primaryTargets.erase(renderTarget_);
}

void RenderSceneManager::transitionToSceneView(mref<smr<Swapchain>> targetKey_, mref<smr<SceneView>> nextView_) {
	const auto iter = _mappedRenderTargets.find(targetKey_);
	assert(iter != _mappedRenderTargets.end());
	[[maybe_unused]] const auto transition = iter->second->transitionToSceneView(std::move(nextView_));
}

void RenderSceneManager::transitionToTarget(mref<smr<Swapchain>> from_, mref<smr<Swapchain>> next_) {
	auto iter = _mappedRenderTargets.find(from_);
	assert(iter != _mappedRenderTargets.end());

	const auto transition = iter->second->transitionToTarget(::hg::move(next_));
	assert(transition.has_value());

	auto renderTarget = std::move(iter->second);
	_mappedRenderTargets.erase(iter);
	_mappedRenderTargets.emplace(transition->pendingSwapchain, std::move(renderTarget));
}

void RenderSceneManager::transitionToTarget(
	ref<Surface> surface_,
	mref<smr<Swapchain>> fromSwapchain_,
	FnRef<smr<Swapchain>(mref<smr<Swapchain>> prev_)> transitionFn_
) {
	auto iter = _mappedRenderTargets.find(::hg::move(fromSwapchain_));
	assert(iter != _mappedRenderTargets.end());

	auto renderTarget = std::move(iter->second);
	_mappedRenderTargets.erase(iter);

	const auto transition = renderTarget->transitionToTarget(surface_, ::hg::move(transitionFn_));
	assert(transition.has_value());

	_mappedRenderTargets.emplace(transition->pendingSwapchain, std::move(renderTarget));
}

void RenderSceneManager::selectInvokeTargets(ref<CompactSet<smr<RenderTarget>>> targets_) const noexcept {
	targets_.reserve(_primaryTargets.size());
	for (const auto& entry : _primaryTargets) {
		if (entry->active() && entry->ready()) {
			targets_.emplace(clone(entry));
		}
	}
}
