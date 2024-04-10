#include "GraphicRegisterImporterEvent.hpp"

using namespace hg;

GraphicRegisterImporterEvent::GraphicRegisterImporterEvent(
	ref<engine::res::ImporterManager> importerManager_,
	ref<engine::Graphics> graphics_
) noexcept :
	StatefulEvent(),
	_manager(std::addressof(importerManager_)),
	_graphics(std::addressof(graphics_)) {}

bool GraphicRegisterImporterEvent::canceled() const noexcept {
	return false;
}

void GraphicRegisterImporterEvent::cancel() noexcept {
	//__noop;
}

ref<engine::res::ImporterManager> GraphicRegisterImporterEvent::getLoaderManager() const noexcept {
	return *_manager.get();
}

ref<engine::Graphics> GraphicRegisterImporterEvent::getGraphics() const noexcept {
	return *_graphics.get();
}
