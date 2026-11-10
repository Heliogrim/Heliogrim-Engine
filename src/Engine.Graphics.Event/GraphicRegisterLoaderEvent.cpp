#include "GraphicRegisterLoaderEvent.hpp"

using namespace hg;

GraphicRegisterLoaderEvent::GraphicRegisterLoaderEvent(
	ref<engine::resource::LoaderManager> loaderManager_,
	ref<engine::Graphics> graphics_,
	ref<engine::graphics::cache::GlobalCacheCtrl> globalCacheCtrl_,
	ref<engine::graphics::pool::GlobalResourcePool> globalResourcePool_
) noexcept :
	StatefulEvent(),
	_manager(std::addressof(loaderManager_)),
	_graphics(std::addressof(graphics_)),
	_cacheCtrl(std::addressof(globalCacheCtrl_)),
	_pool(std::addressof(globalResourcePool_)) {}

bool GraphicRegisterLoaderEvent::canceled() const noexcept {
	return false;
}

void GraphicRegisterLoaderEvent::cancel() noexcept {
	// __noop;
}

ref<engine::resource::LoaderManager> GraphicRegisterLoaderEvent::getLoaderManager() const noexcept {
	return *_manager.get();
}

ref<engine::Graphics> GraphicRegisterLoaderEvent::getGraphics() const noexcept {
	return *_graphics.get();
}

ref<engine::graphics::cache::GlobalCacheCtrl> GraphicRegisterLoaderEvent::getGraphicsGlobalCacheCtrl() const noexcept {
	return *_cacheCtrl.get();
}

ref<engine::graphics::pool::GlobalResourcePool> GraphicRegisterLoaderEvent::getGraphicsGlobalResourcePool() const noexcept {
	return *_pool.get();
}
