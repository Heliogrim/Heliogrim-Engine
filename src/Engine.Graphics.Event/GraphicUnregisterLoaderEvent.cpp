#include "GraphicUnregisterLoaderEvent.hpp"

using namespace hg;

GraphicUnregisterLoaderEvent::GraphicUnregisterLoaderEvent(
	ref<engine::resource::LoaderManager> loaderManager_,
	ref<engine::Graphics> graphics_,
	ref<engine::gfx::cache::GlobalCacheCtrl> globalCacheCtrl_,
	ref<engine::gfx::pool::GlobalResourcePool> globalResourcePool_
) noexcept :
	StatefulEvent(),
	_manager(std::addressof(loaderManager_)),
	_graphics(std::addressof(graphics_)),
	_cacheCtrl(std::addressof(globalCacheCtrl_)),
	_pool(std::addressof(globalResourcePool_)) {}

bool GraphicUnregisterLoaderEvent::canceled() const noexcept {
	return false;
}

void GraphicUnregisterLoaderEvent::cancel() noexcept {
	// __noop;
}

ref<engine::resource::LoaderManager> GraphicUnregisterLoaderEvent::getLoaderManager() const noexcept {
	return *_manager.get();
}

ref<engine::Graphics> GraphicUnregisterLoaderEvent::getGraphics() const noexcept {
	return *_graphics.get();
}

ref<engine::gfx::cache::GlobalCacheCtrl> GraphicUnregisterLoaderEvent::getGraphicsGlobalCacheCtrl() const noexcept {
	return *_cacheCtrl.get();
}

ref<engine::gfx::pool::GlobalResourcePool>
GraphicUnregisterLoaderEvent::getGraphicsGlobalResourcePool() const noexcept {
	return *_pool.get();
}
