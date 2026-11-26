#include <algorithm>
#include <cassert>
#include <Engine.Utils/_CTRL.hpp>

import Engine.Common;
import Engine.Graphics;
import Engine.Graphics.Render;
import Engine.Graphics.Scene;

using namespace hg::engine::graphics::scene;
using namespace hg::engine::graphics;
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

CompactArray<smr<RenderTarget>> RenderSceneManager::snapshotPrimaryTargets() const {
	auto snapshot = CompactArray<smr<RenderTarget>> {};

	const concurrent::SpinLock lock { _targetsMtx };
	snapshot.reserve(_primaryTargets.size());
	for (const auto& entry : _primaryTargets) {
		snapshot.emplace_back(clone(entry));
	}
	return snapshot;
}

void RenderSceneManager::addPrimaryTarget(mref<smr<RenderTarget>> renderTarget_) {
	auto key = renderTarget_->getSwapChain();
	auto copy = clone(renderTarget_);

	const concurrent::SpinLock lock { _targetsMtx };
	_primaryTargets.insert(std::move(copy));
	_mappedRenderTargets.emplace(std::move(key), std::move(renderTarget_));
}

void RenderSceneManager::dropPrimaryTarget(cref<smr<RenderTarget>> renderTarget_) {
	// Moved out so the references die after unlocking.
	auto mapped = smr<RenderTarget> { nullptr };
	auto primary = smr<RenderTarget> { nullptr };

	/**/

	{
		const concurrent::SpinLock lock { _targetsMtx };

		// Key by value: the swapchain key is null or stale while a transition is in flight.
		const auto mappedIter = std::ranges::find_if(
			_mappedRenderTargets,
			[&](const auto& entry) { return entry.second == renderTarget_; }
		);
		if (mappedIter != _mappedRenderTargets.end()) {
			mapped = std::move(mappedIter->second);
			_mappedRenderTargets.erase(mappedIter);
		}

		if (const auto primaryIter = _primaryTargets.find(renderTarget_); primaryIter != _primaryTargets.end()) {
			primary = clone(*primaryIter);
			_primaryTargets.erase(primaryIter);
		}
	}
}

void RenderSceneManager::transitionToSceneView(mref<smr<Swapchain>> targetKey_, mref<smr<SceneView>> nextView_) {
	auto target = smr<RenderTarget> { nullptr };

	/**/

	{
		const concurrent::SpinLock lock { _targetsMtx };
		const auto iter = _mappedRenderTargets.find(targetKey_);
		assert(iter != _mappedRenderTargets.end());
		target = clone(iter->second);
	}
	target->transitionToSceneView(std::move(nextView_));
}

void RenderSceneManager::transitionToTarget(mref<smr<Swapchain>> from_, mref<smr<Swapchain>> next_) {
	auto renderTarget = smr<RenderTarget> { nullptr };

	/**/

	{
		const concurrent::SpinLock lock { _targetsMtx };
		const auto iter = _mappedRenderTargets.find(from_);
		assert(iter != _mappedRenderTargets.end());
		renderTarget = clone(iter->second);
	}

	const auto transition = renderTarget->transitionToTarget(::hg::move(next_));
	assert(transition.has_value());

	// Skip when a concurrent drop or transition took the mapping; renderTarget then dies after the lock.
	const concurrent::SpinLock lock { _targetsMtx };
	const auto iter = _mappedRenderTargets.find(from_);
	if (iter != _mappedRenderTargets.end() && iter->second == renderTarget && _primaryTargets.contains(renderTarget)) {
		_mappedRenderTargets.erase(iter);
		_mappedRenderTargets.emplace(transition->pendingSwapchain, std::move(renderTarget));
	}
}

bool RenderSceneManager::transitionToTarget(
	ref<Surface> surface_,
	mref<smr<Swapchain>> fromSwapchain_,
	FnRef<smr<Swapchain>(mref<smr<Swapchain>> prev_)> transitionFn_
) {
	auto renderTarget = smr<RenderTarget> { nullptr };

	/**/

	{
		const concurrent::SpinLock lock { _targetsMtx };
		const auto iter = _mappedRenderTargets.find(fromSwapchain_);
		assert(iter != _mappedRenderTargets.end());

		renderTarget = std::move(iter->second);
		_mappedRenderTargets.erase(iter);
	}

	const auto transition = renderTarget->transitionToTarget(surface_, ::hg::move(transitionFn_));

	// Re-emplace only if not dropped meanwhile; a dropped renderTarget dies after the lock (scope exit order).
	const auto restored = [&](auto&& key_) {
		const concurrent::SpinLock lock { _targetsMtx };
		if (_primaryTargets.contains(renderTarget)) {
			_mappedRenderTargets.emplace(std::forward<decltype(key_)>(key_), std::move(renderTarget));
		}
	};

	if (not transition.has_value()) {
		restored(::hg::move(fromSwapchain_));
		return false;
	}

	restored(transition->pendingSwapchain);
	return true;
}

CompactArray<smr<RenderTarget>> RenderSceneManager::selectInvokeTargets() const {
	return selectPrimaryTargets([](cref<smr<RenderTarget>> target_) { return target_->active() && target_->ready(); });
}
