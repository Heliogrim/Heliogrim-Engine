#include "GraphicUnregisterImporterEvent.hpp"

using namespace hg;

GraphicUnregisterImporterEvent::GraphicUnregisterImporterEvent(
	ref<engine::res::ImporterManager> importerManager_,
	ref<engine::Graphics> graphics_
) noexcept :
	StatefulEvent(),
	_manager(std::addressof(importerManager_)),
	_graphics(std::addressof(graphics_)) {}

bool GraphicUnregisterImporterEvent::canceled() const noexcept {
	return false;
}

void GraphicUnregisterImporterEvent::cancel() noexcept {
	// __noop;
}

ref<engine::res::ImporterManager> GraphicUnregisterImporterEvent::getLoaderManager() const noexcept {
	return *_manager.get();
}

ref<engine::Graphics> GraphicUnregisterImporterEvent::getGraphics() const noexcept {
	return *_graphics.get();
}
